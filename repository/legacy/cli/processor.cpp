#include "processor.hpp"

#include <algorithm>
#include <cctype>
#include <filesystem>
#include <fstream>
#include <future>
#include <iostream>
#include <sstream>
#include <thread>
#include <vector>

#include "art2img/version.hpp"
#include "cli_operations.hpp"

// =============================================================================

#include "BS_thread_pool.hpp"
#include "art2img/art_file.hpp"
#include "art2img/colors.hpp"
#include "art2img/extractor_api.hpp"
#include "art2img/image_writer.hpp"
#include "art2img/palette.hpp"

namespace {

std::size_t hardware_thread_count() {
  unsigned int count = std::thread::hardware_concurrency();
  return count == 0 ? 1U : static_cast<std::size_t>(count);
}

std::size_t resolve_worker_count(const ProcessingOptions& options, std::size_t total_tiles) {
  std::size_t desired = options.max_threads > 0 ? options.max_threads : hardware_thread_count();
  if (desired == 0) {
    desired = 1;
  }
  if (total_tiles > 0) {
    desired = std::min<std::size_t>(desired, total_tiles);
  }
  return std::max<std::size_t>(desired, 1);
}

bool should_use_parallel(const ProcessingOptions& options, std::size_t total_tiles,
                         bool prefer_parallel) {
  if (!prefer_parallel || !options.enable_parallel) {
    return false;
  }
  return resolve_worker_count(options, total_tiles) > 1;
}

struct TileProcessingContext {
  const art2img::ArtView* art_view = nullptr;
  const ProcessingOptions* options = nullptr;
  const std::string* output_dir = nullptr;
  uint32_t total_tiles = 0;
};

TileResult export_tile(const TileProcessingContext& context, uint32_t local_index) {
  art2img::ImageView image_view{context.art_view, local_index};
  const uint32_t tile_index = local_index + context.art_view->header.start_tile;
  return process_single_tile_composable(image_view, *context.output_dir, *context.options,
                                        tile_index);
}

void log_tile_failure(const TileResult& tile_result) {
  art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
  std::cerr << "Warning: Failed to process tile " << tile_result.tile_index << ": "
            << tile_result.error_message << std::endl;
  std::cerr << "This may be due to file permissions or disk space issues." << std::endl;
}

void log_progress_if_needed(const ProcessingOptions& options, std::size_t completed,
                            std::size_t total_tiles) {
  if (!options.verbose || total_tiles <= 50 || (completed % 10) != 0) {
    return;
  }

  art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
  std::cout << "Progress: " << completed << "/" << total_tiles << " tiles processed"
            << art2img::ColorOutput::reset() << std::endl;
}

void handle_tile_result(const TileResult& tile_result, ProcessingResult& summary,
                        const ProcessingOptions& options, uint32_t total_tiles,
                        std::size_t completed) {
  if (tile_result.success) {
    summary.processed_count++;
  } else {
    summary.failed_count++;
    log_tile_failure(tile_result);
  }

  log_progress_if_needed(options, completed, static_cast<std::size_t>(total_tiles));
}

void process_tiles_sequential(const TileProcessingContext& context, ProcessingResult& result) {
  for (uint32_t i = 0; i < context.total_tiles; ++i) {
    TileResult tile_result = export_tile(context, i);
    handle_tile_result(tile_result, result, *context.options, context.total_tiles,
                       static_cast<std::size_t>(i + 1));
  }
}

void process_tiles_parallel(const TileProcessingContext& context, std::size_t worker_count,
                            ProcessingResult& result) {
  BS::thread_pool pool(worker_count);
  std::vector<std::future<TileResult>> futures;
  futures.reserve(context.total_tiles);

  for (uint32_t i = 0; i < context.total_tiles; ++i) {
    futures.emplace_back(pool.submit_task([context, i]() { return export_tile(context, i); }));
  }

  std::size_t completed = 0;
  for (auto& future : futures) {
    TileResult tile_result = future.get();
    ++completed;
    handle_tile_result(tile_result, result, *context.options, context.total_tiles, completed);
  }
}

void log_processing_summary(const ProcessingOptions& options, const ProcessingResult& result) {
  if (!options.verbose) {
    return;
  }

  if (result.failed_count == 0) {
    art2img::ColorGuard green(art2img::ColorOutput::GREEN);
    std::cout << "Tile processing complete: " << result.processed_count << " successful"
              << art2img::ColorOutput::reset() << std::endl;
  } else {
    art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW);
    std::cout << "Tile processing complete: " << result.processed_count << " successful, "
              << result.failed_count << " failed" << art2img::ColorOutput::reset() << std::endl;
  }
}

void write_animation_data_if_requested(art2img::ExtractorAPI& extractor,
                                       const ProcessingOptions& options,
                                       const std::string& art_file_path, bool is_directory_mode) {
  if (!((options.dump_animation && !is_directory_mode) ||
        (options.merge_animation_data && is_directory_mode))) {
    return;
  }

  if (!extractor.write_animation_data(art_file_path, options.output_dir) && !is_directory_mode) {
    std::cerr << "Warning: Failed to write animation data for " << art_file_path << std::endl;
  }
}

}  // namespace

// Composable function to load ART file and palette
LoadedArtData load_art_and_palette_composable(const ProcessingOptions& options,
                                              const std::string& art_file_path) {
  LoadedArtData result;

  try {
    // Load palette with automatic fallback
    art2img::Palette palette;
    if (!load_palette_with_fallback(palette, options, art_file_path)) {
      result.error_message = "Failed to load palette";
      return result;
    }

    // Load ART file through ExtractorAPI
    auto extractor = std::make_unique<art2img::ExtractorAPI>();
    if (!extractor->load_art_file(art_file_path)) {
      result.error_message = "Failed to load ART file: " + art_file_path;
      result.error_message +=
          " (Please check that the file exists and is a valid Duke Nukem 3D ART file)";
      return result;
    }

    if (!extractor->load_palette_from_memory(palette.raw_data().data(),
                                             palette.raw_data().size())) {
      result.error_message = "Failed to load palette data";
      return result;
    }

    result.extractor = std::move(extractor);
    result.success = true;
  } catch (const art2img::ArtException& e) {
    result.error_message = std::string("ART Exception: ") + e.what();
  } catch (const std::exception& e) {
    result.error_message = std::string("Exception: ") + e.what();
  } catch (...) {
    result.error_message = "Unknown exception while loading";
  }

  return result;
}

// Composable function to save image in specific format
bool save_image_format(const art2img::ImageView& image_view, const std::string& filepath,
                       const std::string& format, bool fix_transparency) {
  // Normalize format string to handle potential Windows encoding issues
  std::string normalized_format = format;
  std::transform(normalized_format.begin(), normalized_format.end(), normalized_format.begin(),
                 [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

  if (normalized_format == "tga") {
    return image_view.save_to_tga(filepath);
  } else if (normalized_format == "bmp") {
    art2img::ImageWriter::Options bmp_options;
    bmp_options.fix_transparency = fix_transparency;
    return image_view.save_to_image(filepath, art2img::ImageFormat::BMP, bmp_options);
  } else if (normalized_format == "png" || normalized_format.empty()) {  // default to PNG
    art2img::ImageWriter::Options png_options;
    png_options.fix_transparency = fix_transparency;
    return image_view.save_to_png(filepath, png_options);
  } else {
    // Fallback to PNG for unknown formats
    art2img::ImageWriter::Options png_options;
    png_options.fix_transparency = fix_transparency;
    return image_view.save_to_png(filepath, png_options);
  }
}

// Composable function to process a single tile
TileResult process_single_tile_composable(const art2img::ImageView& image_view,
                                          const std::string& output_dir,
                                          const ProcessingOptions& options, uint32_t tile_index) {
  TileResult result;
  result.tile_index = tile_index;

  if (image_view.pixel_data() == nullptr || image_view.width() == 0 || image_view.height() == 0) {
    result.success = true;  // Empty tiles succeed by definition
    return result;
  }

  // Construct output filename
  std::string tile_filename =
      output_dir + "/tile" + std::to_string(tile_index) + "." + options.format;

  // Save based on format using streamlined function
  result.success =
      save_image_format(image_view, tile_filename, options.format, options.fix_transparency);
  result.output_path = tile_filename;

  if (!result.success) {
    result.error_message = "Failed to save tile " + std::to_string(tile_index);
  }

  return result;
}

namespace {

ProcessingResult process_art_file_internal(const ProcessingOptions& options,
                                           const std::string& art_file_path,
                                           const std::string& output_subdir, bool is_directory_mode,
                                           bool prefer_parallel) {
  ProcessingResult result;

  try {
    if (options.verbose) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "Processing ART file: " << art_file_path << std::endl;
    }

    auto loaded_data = load_art_and_palette_composable(options, art_file_path);
    if (!loaded_data.success) {
      result.success = false;
      result.error_message = loaded_data.error_message;
      return result;
    }

    std::string final_output_dir = options.output_dir;
    if (!output_subdir.empty()) {
      final_output_dir = (std::filesystem::path(final_output_dir) / output_subdir).string();
    }

    if (!create_output_directories(final_output_dir)) {
      result.success = false;
      result.error_message = "Failed to create output directory: " + final_output_dir;
      return result;
    }

    auto art_view = loaded_data.extractor->get_art_view();
    const uint32_t total_tiles = art_view.image_count();

    if (options.verbose) {
      std::cout << "Processing " << total_tiles << " tiles..." << std::endl;
    }

    const bool run_parallel = should_use_parallel(options, total_tiles, prefer_parallel);
    const std::size_t worker_count =
        run_parallel ? resolve_worker_count(options, total_tiles) : static_cast<std::size_t>(1);

    if (run_parallel && options.verbose) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "Using parallel export with " << worker_count << " worker threads"
                << art2img::ColorOutput::reset() << std::endl;
    }

    TileProcessingContext context{&art_view, &options, &final_output_dir, total_tiles};

    if (run_parallel) {
      process_tiles_parallel(context, worker_count, result);
    } else {
      process_tiles_sequential(context, result);
    }

    log_processing_summary(options, result);

    write_animation_data_if_requested(*loaded_data.extractor, options, art_file_path,
                                      is_directory_mode);

    result.success = (result.failed_count == 0) || (result.processed_count > 0);
  } catch (const art2img::ArtException& e) {
    result.success = false;
    result.error_message = std::string("ART Exception: ") + e.what();
  } catch (const std::exception& e) {
    result.success = false;
    result.error_message = std::string("Exception: ") + e.what();
  } catch (...) {
    result.success = false;
    result.error_message = "Unknown exception occurred while processing " + art_file_path;
  }

  return result;
}

}  // namespace

ProcessingResult process_sequential_impl(const ProcessingOptions& options,
                                         const std::string& art_file_path,
                                         const std::string& output_subdir, bool is_directory_mode) {
  return process_art_file_internal(options, art_file_path, output_subdir, is_directory_mode, false);
}

ProcessingResult process_parallel_impl(const ProcessingOptions& options,
                                       const std::string& art_file_path,
                                       const std::string& output_subdir, bool is_directory_mode) {
  return process_art_file_internal(options, art_file_path, output_subdir, is_directory_mode, true);
}

// Composable function to find palette file
std::optional<std::string> find_palette_file(const std::string& user_path,
                                             const std::string& art_file_path) {
  // Try user-specified path first
  if (!user_path.empty()) {
    std::error_code ec;
    if (std::filesystem::exists(user_path, ec)) {
      return user_path;
    }
  }

  // Try adjacent to ART file
  const std::filesystem::path art_dir = std::filesystem::path(art_file_path).parent_path();
  if (!art_dir.empty()) {
    std::vector<std::string> candidates = {(art_dir / "palette.dat").string(),
                                           (art_dir / "PALETTE.DAT").string()};

    for (const auto& candidate : candidates) {
      std::error_code ec;
      if (std::filesystem::exists(candidate, ec)) {
        return candidate;
      }
    }
  }

  // Try common locations
  std::vector<std::string> common_locations = {"palette.dat", "PALETTE.DAT", "assets/palette.dat",
                                               "assets/PALETTE.DAT"};

  for (const auto& candidate : common_locations) {
    std::error_code ec;
    if (std::filesystem::exists(candidate, ec)) {
      return candidate;
    }
  }

  return std::nullopt;
}

// Composable function to load palette with fallback
bool load_palette_with_fallback(art2img::Palette& palette, const ProcessingOptions& options,
                                const std::string& art_file_path) {
  auto palette_path = find_palette_file(options.palette_file, art_file_path);

  // Try to load from found path
  if (palette_path.has_value()) {
    try {
      if (palette.load_from_file(palette_path.value())) {
        if (options.verbose) {
          std::cout << "Using palette file: " << palette_path.value() << std::endl;
        }
        return true;
      }
    } catch (const art2img::ArtException& e) {
      if (options.verbose) {
        std::cout << "Warning: " << e.what() << std::endl;
        std::cout << "Falling back to default palette..." << std::endl;
      }
    }
  }

  // Show warning if needed
  if (options.verbose) {
    if (palette_path.has_value()) {
      std::cout << "Warning: Cannot open palette file '" << palette_path.value() << "'"
                << std::endl;
    } else if (!options.palette_file.empty()) {
      std::cout << "Warning: Cannot locate palette file '" << options.palette_file << "'"
                << std::endl;
    } else {
      std::cout << "Info: No palette file specified, using default Duke Nukem 3D palette"
                << std::endl;
    }
    std::cout << "Using built-in Duke Nukem 3D palette (256 colors)" << std::endl;
  }

  // Load default palette
  palette.load_duke3d_default();
  return true;
}

// Composable function to ensure output directory exists
bool create_output_directories(const std::string& output_dir) {
  if (output_dir.empty())
    return true;

  std::error_code dir_error;
  std::filesystem::create_directories(output_dir, dir_error);

  if (dir_error) {
    art2img::ColorGuard red(art2img::ColorOutput::RED, std::cerr);
    std::cerr << "Error: Failed to create output directory '" << output_dir
              << "': " << dir_error.message() << art2img::ColorOutput::reset() << std::endl;
    art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
    std::cerr << "Please ensure you have write permissions to the parent directory and that the "
                 "path is valid."
              << art2img::ColorOutput::reset() << std::endl;
    return false;
  }
  return true;
}

// Processing router that can switch between modes
ProcessingResult process_with_mode(const ProcessingOptions& options,
                                   const std::string& art_file_path,
                                   const std::string& output_subdir, bool is_directory_mode,
                                   bool use_parallel) {
  if (use_parallel) {
    return process_parallel_impl(options, art_file_path, output_subdir, is_directory_mode);
  } else {
    return process_sequential_impl(options, art_file_path, output_subdir, is_directory_mode);
  }
}

// Updated process_single_art_file using composable architecture
ProcessingResult process_single_art_file(const ProcessingOptions& options,
                                         const std::string& art_file_path,
                                         const std::string& output_subdir, bool is_directory_mode) {
  return process_with_mode(options, art_file_path, output_subdir, is_directory_mode,
                           options.enable_parallel);
}

/// Process all ART files in a directory
CliProcessResult process_art_directory(const CliOptions& cli_options,
                                       const ProcessingOptions& options) {
  CliProcessResult cli_result;

  if (options.verbose) {
    art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
    std::cout << "Processing ART files in directory: " << cli_options.input_path
              << art2img::ColorOutput::reset() << std::endl;
  }

  if (options.merge_animation_data) {
    std::string merged_ini_path =
        (std::filesystem::path(options.output_dir) / "animdata.ini").string();
    std::ofstream merged_file(merged_ini_path);
    if (merged_file.is_open()) {
      merged_file << "; Merged animation data from all ART files\n"
                  << "; Extracted by art2img v" << ART2IMG_VERSION << "\n"
                  << "; Generated: " << __DATE__ << " " << __TIME__ << "\n"
                  << "\n";
      if (options.verbose) {
        std::cout << "Created merged animation data file: " << merged_ini_path << std::endl;
      }
    } else {
      art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
      std::cerr << "Warning: Failed to create merged animation data file"
                << art2img::ColorOutput::reset() << std::endl;
      std::cerr << "Please ensure you have write permissions to the output directory."
                << art2img::ColorOutput::reset() << std::endl;
    }
  }

  // Collect all ART files
  std::vector<std::string> art_files;
  try {
    for (const auto& entry : std::filesystem::directory_iterator(cli_options.input_path)) {
      if (entry.is_regular_file()) {
        std::string extension = entry.path().extension().string();
        if (extension == ".art" || extension == ".ART") {
          art_files.push_back(entry.path().string());
        }
      }
    }
  } catch (const std::filesystem::filesystem_error& e) {
    cli_result.error_message =
        std::string("Failed to read directory '") + cli_options.input_path + "': " + e.what();
    return cli_result;
  }

  std::sort(art_files.begin(), art_files.end());

  if (art_files.empty()) {
    cli_result.error_message =
        std::string("No ART files found in directory '") + cli_options.input_path + "'.";
    return cli_result;
  }

  if (!cli_options.quiet) {
    art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
    std::cout << "Found " << art_files.size() << " ART files to process"
              << art2img::ColorOutput::reset() << std::endl;
  }

  // Process each ART file
  size_t processed_files = 0;
  size_t successful_files = 0;
  std::string first_error_message;

  for (const auto& art_file : art_files) {
    ++processed_files;
    if (!cli_options.quiet) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "Processing file " << processed_files << "/" << art_files.size() << ": "
                << std::filesystem::path(art_file).filename() << art2img::ColorOutput::reset()
                << std::endl;
    }

    const std::string subdir = std::filesystem::path(art_file).stem().string();
    auto file_result = process_single_art_file(options, art_file, subdir, true);

    if (file_result.success) {
      ++successful_files;
    } else if (first_error_message.empty()) {
      if (!file_result.error_message.empty()) {
        first_error_message = file_result.error_message;
      } else {
        first_error_message = std::string("Failed to process '") + art_file + "'.";
      }
    }
  }

  if (options.verbose) {
    std::cout << "\nDirectory processing complete: " << successful_files << "/" << processed_files
              << " files successful" << std::endl;
    if (successful_files == processed_files) {
      art2img::ColorGuard green(art2img::ColorOutput::GREEN);
      std::cout << "All files processed successfully!" << art2img::ColorOutput::reset()
                << std::endl;
    } else if (successful_files > 0) {
      art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW);
      std::cout << "Some files processed with warnings." << art2img::ColorOutput::reset()
                << std::endl;
    }
  }

  cli_result.success = (processed_files > 0) && (successful_files == processed_files);

  if (!cli_result.success) {
    if (successful_files == 0) {
      cli_result.error_message =
          first_error_message.empty()
              ? std::string("Failed to process any ART files in directory '") +
                    cli_options.input_path + "'."
              : first_error_message;
    } else {
      std::ostringstream oss;
      oss << "Processed " << successful_files << " of " << processed_files
          << " ART files with errors.";
      cli_result.error_message = first_error_message.empty() ? oss.str() : first_error_message;
    }
  }

  return cli_result;
}

/// Wrapper function to call process_single_art_file with command line options
CliProcessResult process_single_art_file_wrapper(const CliOptions& cli_options,
                                                 const ProcessingOptions& options) {
  auto processing_result = process_single_art_file(options, cli_options.input_path, "", false);

  CliProcessResult cli_result;
  cli_result.success = processing_result.success;

  if (!processing_result.success) {
    if (!processing_result.error_message.empty()) {
      cli_result.error_message = processing_result.error_message;
    } else if (processing_result.failed_count > 0) {
      std::ostringstream oss;
      oss << "Processed " << processing_result.processed_count << " tile(s) with "
          << processing_result.failed_count << " failure(s) in '" << cli_options.input_path << "'.";
      cli_result.error_message = oss.str();
    } else {
      cli_result.error_message =
          std::string("Failed to process ART file '") + cli_options.input_path + "'.";
    }
  }

  return cli_result;
}
