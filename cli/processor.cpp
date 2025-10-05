#include "processor.hpp"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>

#include "cli_operations.hpp"
#include "version.hpp"

// =============================================================================

#include "art_file.hpp"
#include "colors.hpp"
#include "extractor_api.hpp"
#include "image_writer.hpp"
#include "palette.hpp"

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
  if (format == "tga") {
    return image_view.save_to_tga(filepath);
  } else if (format == "bmp") {
    art2img::ImageWriter::Options bmp_options;
    bmp_options.fix_transparency = fix_transparency;
    return image_view.save_to_image(filepath, art2img::ImageFormat::BMP, bmp_options);
  } else {  // default to PNG
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

// Sequential processing implementation
ProcessingResult process_sequential_impl(const ProcessingOptions& options,
                                         const std::string& art_file_path,
                                         const std::string& output_subdir, bool is_directory_mode) {
  ProcessingResult result;

  try {
    if (options.verbose) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "Processing ART file: " << art_file_path << std::endl;
    }

    // Load data using composable function
    auto loaded_data = load_art_and_palette_composable(options, art_file_path);
    if (!loaded_data.success) {
      result.success = false;
      result.error_message = loaded_data.error_message;
      return result;
    }

    // Determine final output directory
    std::string final_output_dir = options.output_dir;
    if (!output_subdir.empty()) {
      final_output_dir = (std::filesystem::path(final_output_dir) / output_subdir).string();
    }

    // Ensure output directory exists
    if (!create_output_directories(final_output_dir)) {
      result.success = false;
      result.error_message = "Failed to create output directory: " + final_output_dir;
      return result;
    }

    // Get art view for processing
    auto art_view = loaded_data.extractor->get_art_view();
    uint32_t total_tiles = art_view.image_count();

    if (options.verbose) {
      std::cout << "Processing " << total_tiles << " tiles..." << std::endl;
    }

    // Process all tiles sequentially using composable function
    for (uint32_t i = 0; i < total_tiles; ++i) {
      art2img::ImageView image_view{&art_view, i};
      uint32_t tile_index = i + art_view.header.start_tile;

      auto tile_result =
          process_single_tile_composable(image_view, final_output_dir, options, tile_index);

      if (tile_result.success) {
        result.processed_count++;
      } else {
        result.failed_count++;
        if (image_view.pixel_data() != nullptr) {  // Only error for non-empty tiles
          art2img::ColorGuard yellow(art2img::ColorOutput::YELLOW, std::cerr);
          std::cerr << "Warning: Failed to process tile " << tile_index << ": "
                    << tile_result.error_message << std::endl;
          std::cerr << "This may be due to file permissions or disk space issues." << std::endl;
        }
      }

      // Show progress for large files
      if (options.verbose && total_tiles > 50 && (i + 1) % 10 == 0) {
        art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
        std::cout << "Progress: " << (i + 1) << "/" << total_tiles << " tiles processed"
                  << art2img::ColorOutput::reset() << std::endl;
      }
    }

    if (options.verbose) {
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

    // Handle animation data output
    if ((options.dump_animation && !is_directory_mode) ||
        (options.merge_animation_data && is_directory_mode)) {
      if (!loaded_data.extractor->write_animation_data(art_file_path, options.output_dir)) {
        if (!is_directory_mode) {
          std::cerr << "Warning: Failed to write animation data for " << art_file_path << std::endl;
        }
      }
    }

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

// Parallel processing stub (for future implementation)
ProcessingResult process_parallel_impl(const ProcessingOptions& options,
                                       const std::string& art_file_path,
                                       const std::string& output_subdir, bool is_directory_mode) {
  // For now, delegate to sequential implementation
  // Future: Implement with thread pool or parallel algorithms
  if (options.verbose) {
    std::cout << "Note: Parallel processing not yet implemented, using sequential mode"
              << std::endl;
  }
  return process_sequential_impl(options, art_file_path, output_subdir, is_directory_mode);
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
bool process_single_art_file(const ProcessingOptions& options, const std::string& art_file_path,
                             const std::string& output_subdir, bool is_directory_mode) {
  // For now, always use sequential mode
  auto result = process_with_mode(options, art_file_path, output_subdir, is_directory_mode, false);
  return result.success;
}

/// Process all ART files in a directory
bool process_art_directory(const CliOptions& cli_options) {
  if (!cli_options.quiet) {
    art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
    std::cout << "Processing ART files in directory: " << cli_options.input_path
              << art2img::ColorOutput::reset() << std::endl;
  }

  // Create merged animation data file if needed
  if (cli_options.merge_anim) {
    std::string merged_ini_path =
        (std::filesystem::path(cli_options.output_dir) / "animdata.ini").string();
    std::ofstream merged_file(merged_ini_path);
    if (merged_file.is_open()) {
      merged_file << "; Merged animation data from all ART files\n"
                  << "; Extracted by art2img v" << ART2IMG_VERSION << "\n"
                  << "; Generated: " << __DATE__ << " " << __TIME__ << "\n"
                  << "\n";
      if (!cli_options.quiet) {
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

  // Prepare processing options
  ProcessingOptions options;
  options.palette_file = cli_options.palette_file;
  options.output_dir = cli_options.output_dir;
  options.format = cli_options.format;
  options.fix_transparency = cli_options.fix_transparency;
  options.verbose = !cli_options.quiet;
  options.dump_animation = !cli_options.no_anim;
  options.merge_animation_data = cli_options.merge_anim;

  // Process each ART file
  int processed_files = 0;
  int successful_files = 0;
  std::vector<std::string> art_files;

  // First, collect all ART files
  for (const auto& entry : std::filesystem::directory_iterator(cli_options.input_path)) {
    if (entry.is_regular_file()) {
      std::string extension = entry.path().extension().string();
      if (extension == ".art" || extension == ".ART") {
        art_files.push_back(entry.path().string());
      }
    }
  }

  if (!cli_options.quiet) {
    art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
    std::cout << "Found " << art_files.size() << " ART files to process"
              << art2img::ColorOutput::reset() << std::endl;
  }

  // Process each ART file
  for (const auto& art_file : art_files) {
    processed_files++;
    if (!cli_options.quiet) {
      art2img::ColorGuard cyan(art2img::ColorOutput::CYAN);
      std::cout << "Processing file " << processed_files << "/" << art_files.size() << ": "
                << std::filesystem::path(art_file).filename() << art2img::ColorOutput::reset()
                << std::endl;
    }

    // Use filename without extension as subdirectory
    std::string subdir = std::filesystem::path(art_file).stem().string();
    if (process_single_art_file(options, art_file, subdir, true)) {
      successful_files++;
    }
  }

  if (!cli_options.quiet) {
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

  return processed_files > 0 && successful_files == processed_files;
}

/// Wrapper function to call process_single_art_file with command line options
bool process_single_art_file_wrapper(const CliOptions& cli_options) {
  ProcessingOptions options;
  options.palette_file = cli_options.palette_file;
  options.output_dir = cli_options.output_dir;
  options.format = cli_options.format;
  options.fix_transparency = cli_options.fix_transparency;
  options.verbose = !cli_options.quiet;
  options.dump_animation = !cli_options.no_anim;
  options.merge_animation_data = cli_options.merge_anim;

  return process_single_art_file(options, cli_options.input_path, "", false);
}