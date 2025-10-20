#include <algorithm>
#include <filesystem>
#include <format>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

#include <CLI/CLI.hpp>

// Include the new art2img API
#include <art2img/api.hpp>

using namespace art2img;

/**
 * @brief CLI configuration structure
 */
struct CliConfig {
  std::string input_path;
  std::string output_dir = ".";
  std::string palette_file;
  std::string format = "png";
  bool fix_transparency = true;
  bool quiet = false;
  bool verbose = false;
  bool parallel = true;
  size_t jobs = 0;       // 0 means auto-detect
  int shade_index = -1;  // -1 means no shading
  bool apply_lookup = true;
  bool premultiply_alpha = false;
  bool matte_hygiene = false;
  bool export_animation = false;
  std::string anim_ini_filename = "animdata.ini";
  bool include_non_animated_tiles = true;
};

/**
 * @brief Progress tracking for parallel processing
 */
struct ProgressTracker {
  std::mutex mutex;
  size_t total = 0;
  size_t completed = 0;
  size_t failed = 0;

  void update(size_t completed_delta = 0, size_t failed_delta = 0) {
    std::lock_guard<std::mutex> lock(mutex);
    completed += completed_delta;
    failed += failed_delta;
  }

  void print_progress() {
    std::lock_guard<std::mutex> lock(mutex);
    if (total > 0) {
      double percent =
          (static_cast<double>(completed + failed) / total) * 100.0;
      std::cout << std::format(
          "\rProgress: {}/{} ({:.1f}%) - Completed: {}, Failed: {}",
          completed + failed, total, percent, completed, failed);
      std::cout.flush();
    }
  }
};

/**
 * @brief Convert ImageFormat enum to string
 */
std::string image_format_to_string(ImageFormat format) {
  switch (format) {
    case ImageFormat::png:
      return "png";
    case ImageFormat::tga:
      return "tga";
    case ImageFormat::bmp:
      return "bmp";
    default:
      return "png";
  }
}

/**
 * @brief Parse string to ImageFormat enum
 */
std::expected<ImageFormat, std::string> parse_format(
    const std::string& format_str) {
  std::string lower_format = format_str;
  std::transform(lower_format.begin(), lower_format.end(), lower_format.begin(),
                 ::tolower);

  if (lower_format == "png")
    return ImageFormat::png;
  if (lower_format == "tga")
    return ImageFormat::tga;
  if (lower_format == "bmp")
    return ImageFormat::bmp;

  return std::unexpected("Unsupported format: " + format_str +
                         ". Supported formats: png, tga, bmp");
}

/**
 * @brief Process a single tile
 */
std::expected<std::monostate, Error> process_tile(
    const TileView& tile, const Palette& palette,
    const ConversionOptions& conv_opts, ImageFormat format,
    const std::filesystem::path& output_path) {
  if (!tile.is_valid()) {
    return make_success();
  }

  // Convert tile to RGBA
  auto image_result = to_rgba(tile, palette, conv_opts);
  if (!image_result) {
    return std::unexpected(image_result.error());
  }

  // Create image view
  auto view = image_view(*image_result);

  // Encode image
  auto encoded_result = encode_image(view, format);
  if (!encoded_result) {
    return std::unexpected(encoded_result.error());
  }

  // Write to file
  auto write_result = write_binary_file(output_path, *encoded_result);
  if (!write_result) {
    return std::unexpected(write_result.error());
  }

  return std::monostate{};
}

/**
 * @brief Process animation export for a single ART file
 */
int process_animation_export(const CliConfig& config) {
  // Load palette
  auto palette_result = load_palette(config.palette_file);
  if (!palette_result) {
    std::cerr << "Error loading palette: " << palette_result.error().message
              << std::endl;
    return 1;
  }

  // Load ART bundle
  auto art_result = load_art_bundle(config.input_path);
  if (!art_result) {
    std::cerr << "Error loading ART file: " << art_result.error().message
              << std::endl;
    return 1;
  }

  // Parse output format
  auto format_result = parse_format(config.format);
  if (!format_result) {
    std::cerr << format_result.error() << std::endl;
    return 1;
  }

  // Create output directory if it doesn't exist
  std::filesystem::create_directories(config.output_dir);

  // Get base filename without extension
  std::filesystem::path input_path(config.input_path);
  std::string base_name = input_path.stem().string();

  // Setup animation export configuration
  AnimationExportConfig anim_config;
  anim_config.output_dir = config.output_dir;
  anim_config.base_name = base_name;
  anim_config.include_non_animated = config.include_non_animated_tiles;
  anim_config.generate_ini = true;
  anim_config.ini_filename = config.anim_ini_filename;
  anim_config.image_format = *format_result;
  anim_config.include_image_references = true;

  if (config.verbose) {
    std::cout << std::format("Exporting animation data from {}...",
                             input_path.filename().string())
              << std::endl;
    std::cout << std::format("Output directory: {}", config.output_dir)
              << std::endl;
    std::cout << std::format("INI filename: {}", config.anim_ini_filename)
              << std::endl;
    std::cout << std::format("Include non-animated tiles: {}",
                             config.include_non_animated_tiles ? "Yes" : "No")
              << std::endl;
  }

  // Export animation data
  auto export_result = export_animation_data(*art_result, anim_config);
  if (!export_result) {
    std::cerr << "Error exporting animation data: "
              << export_result.error().message << std::endl;
    return 1;
  }

  if (config.verbose) {
    std::cout << "Animation export completed successfully" << std::endl;
  }

  return 0;
}

/**
 * @brief Process a single ART file
 */
int process_art_file(const CliConfig& config, ProgressTracker& progress) {
  // Load palette
  auto palette_result = load_palette(config.palette_file);
  if (!palette_result) {
    std::cerr << "Error loading palette: " << palette_result.error().message
              << std::endl;
    return 1;
  }

  // Load ART bundle
  auto art_result = load_art_bundle(config.input_path);
  if (!art_result) {
    std::cerr << "Error loading ART file: " << art_result.error().message
              << std::endl;
    return 1;
  }

  // Parse output format
  auto format_result = parse_format(config.format);
  if (!format_result) {
    std::cerr << format_result.error() << std::endl;
    return 1;
  }
  ImageFormat output_format = *format_result;

  // Setup conversion options
  ConversionOptions conv_opts{};
  conv_opts.apply_lookup = config.apply_lookup;
  conv_opts.fix_transparency = config.fix_transparency;
  conv_opts.premultiply_alpha = config.premultiply_alpha;
  conv_opts.matte_hygiene = config.matte_hygiene;
  if (config.shade_index >= 0) {
    conv_opts.shade_index = static_cast<uint8_t>(config.shade_index);
  }

  // Create output directory if it doesn't exist
  std::filesystem::create_directories(config.output_dir);

  // Get base filename without extension
  std::filesystem::path input_path(config.input_path);
  std::string base_name = input_path.stem().string();

  progress.total = art_result->tiles.size();

  if (config.verbose) {
    std::cout << std::format("Processing {} tiles from {}...",
                             art_result->tiles.size(),
                             input_path.filename().string())
              << std::endl;
  }

  // Process tiles
  if (config.parallel && config.jobs != 1) {
    // Parallel processing
    size_t num_threads =
        config.jobs > 0 ? config.jobs : std::thread::hardware_concurrency();
    std::vector<std::thread> threads;
    std::mutex tiles_mutex;
    size_t current_tile = 0;

    for (size_t t = 0; t < num_threads; ++t) {
      threads.emplace_back([&]() {
        while (true) {
          size_t tile_index;
          {
            std::lock_guard<std::mutex> lock(tiles_mutex);
            if (current_tile >= art_result->tiles.size())
              break;
            tile_index = current_tile++;
          }

          const auto& tile = art_result->tiles[tile_index];
          std::string output_filename =
              std::format("{}_{:04d}.{}", base_name, tile_index,
                          image_format_to_string(output_format));
          std::filesystem::path output_path =
              std::filesystem::path(config.output_dir) / output_filename;

          auto result = process_tile(tile, *palette_result, conv_opts,
                                     output_format, output_path);
          if (result) {
            progress.update(1, 0);
          } else {
            progress.update(0, 1);
            if (!config.quiet) {
              std::lock_guard<std::mutex> lock(progress.mutex);
              std::cerr << std::format("\nError processing tile {}: {}",
                                       tile_index, result.error().message)
                        << std::endl;
            }
          }

          if (config.verbose) {
            progress.print_progress();
          }
        }
      });
    }

    for (auto& thread : threads) {
      thread.join();
    }
  } else {
    // Sequential processing
    for (size_t i = 0; i < art_result->tiles.size(); ++i) {
      const auto& tile = art_result->tiles[i];
      std::string output_filename = std::format(
          "{}_{:04d}.{}", base_name, i, image_format_to_string(output_format));
      std::filesystem::path output_path =
          std::filesystem::path(config.output_dir) / output_filename;

      auto result = process_tile(tile, *palette_result, conv_opts,
                                 output_format, output_path);
      if (result) {
        progress.update(1, 0);
      } else {
        progress.update(0, 1);
        if (!config.quiet) {
          std::cerr << std::format("Error processing tile {}: {}", i,
                                   result.error().message)
                    << std::endl;
        }
      }

      if (config.verbose) {
        progress.print_progress();
      }
    }
  }

  if (config.verbose) {
    std::cout << std::endl;
  }

  return 0;
}

/**
 * @brief Process a directory of ART files
 */
int process_art_directory(const CliConfig& config) {
  int exit_code = 0;
  ProgressTracker total_progress;

  // Count total ART files
  for (const auto& entry :
       std::filesystem::directory_iterator(config.input_path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".ART") {
      total_progress.total++;
    }
  }

  if (total_progress.total == 0) {
    std::cerr << "No ART files found in directory: " << config.input_path
              << std::endl;
    return 1;
  }

  if (config.verbose) {
    std::cout << std::format("Found {} ART files in directory: {}",
                             total_progress.total, config.input_path)
              << std::endl;
  }

  // Process each ART file
  size_t processed = 0;
  for (const auto& entry :
       std::filesystem::directory_iterator(config.input_path)) {
    if (entry.is_regular_file() && entry.path().extension() == ".ART") {
      CliConfig file_config = config;
      file_config.input_path = entry.path().string();

      if (config.verbose) {
        std::cout << std::format("\nProcessing file {}/{}: {}", ++processed,
                                 total_progress.total,
                                 entry.path().filename().string())
                  << std::endl;
      }

      ProgressTracker file_progress;
      int result = process_art_file(file_config, file_progress);
      if (result != 0) {
        exit_code = result;
      }

      if (!config.quiet) {
        std::cout << std::format("File {}: {} tiles completed, {} tiles failed",
                                 entry.path().filename().string(),
                                 file_progress.completed, file_progress.failed)
                  << std::endl;
      }
    }
  }

  return exit_code;
}

int main(int argc, char* argv[]) {
  CLI::App app{"art2img v1.0.0 - Duke Nukem 3D ART File Converter"};

  CliConfig config;

  // Required arguments
  app.add_option("input", config.input_path,
                 "Input ART file or directory containing ART files")
      ->required();

  // Optional arguments
  app.add_option("-o,--output", config.output_dir,
                 "Output directory (default: current directory)")
      ->capture_default_str();

  app.add_option("-p,--palette", config.palette_file,
                 "Palette file path (default: auto-detect)")
      ->capture_default_str();

  app.add_option("-f,--format", config.format,
                 "Output format: png, tga, bmp (default: png)")
      ->capture_default_str();

  // Conversion options
  app.add_flag("--no-transparency-fix", config.fix_transparency,
               "Disable transparency fix")
      ->capture_default_str();

  app.add_option("--shade", config.shade_index,
                 "Apply shade table index (-1 to disable)")
      ->capture_default_str();

  app.add_flag("--no-lookup", config.apply_lookup,
               "Disable lookup table application")
      ->capture_default_str();

  app.add_flag("--premultiply-alpha", config.premultiply_alpha,
               "Premultiply alpha channel")
      ->capture_default_str();

  app.add_flag("--matte-hygiene", config.matte_hygiene,
               "Apply matte hygiene (erosion + blur) to remove halo effects")
      ->capture_default_str();

  // Threading options
  app.add_flag("--no-parallel", config.parallel, "Disable parallel processing")
      ->capture_default_str();

  app.add_option("-j,--jobs", config.jobs,
                 "Number of parallel jobs (0 for auto-detect)")
      ->capture_default_str();

  // Animation export options
  app.add_flag("--export-animation", config.export_animation,
               "Export animation data instead of individual tiles")
      ->capture_default_str();

  app.add_option("--anim-ini-filename", config.anim_ini_filename,
                 "INI filename for animation data (default: animdata.ini)")
      ->capture_default_str();

  app.add_flag("--include-non-animated-tiles",
               config.include_non_animated_tiles,
               "Include non-animated tiles in animation export (default: true)")
      ->capture_default_str();

  // Verbosity
  app.add_flag("-q,--quiet", config.quiet, "Suppress non-error output")
      ->capture_default_str();

  app.add_flag("-v,--verbose", config.verbose, "Verbose output")
      ->capture_default_str();

  // Version flag
  app.set_version_flag("--version", "art2img v1.0.0");

  CLI11_PARSE(app, argc, argv);

  // Validate input path
  if (!std::filesystem::exists(config.input_path)) {
    std::cerr << "Error: Input path does not exist: " << config.input_path
              << std::endl;
    return 1;
  }

  // Set default palette if not provided
  if (config.palette_file.empty()) {
    // Try to find PALETTE.DAT in the same directory as input
    std::filesystem::path input_dir;
    if (std::filesystem::is_directory(config.input_path)) {
      input_dir = std::filesystem::path(config.input_path);
    } else {
      input_dir = std::filesystem::path(config.input_path).parent_path();
    }
    std::filesystem::path default_palette = input_dir / "PALETTE.DAT";

    if (std::filesystem::exists(default_palette)) {
      config.palette_file = default_palette.string();
      if (config.verbose) {
        std::cout << "Using auto-detected palette: " << config.palette_file
                  << std::endl;
      }
    } else {
      std::cerr << "Warning: No palette file specified and PALETTE.DAT not "
                   "found in input directory"
                << std::endl;
      std::cerr << "Use -p/--palette to specify a palette file" << std::endl;
      return 1;
    }
  }

  // Validate palette file
  if (!std::filesystem::exists(config.palette_file)) {
    std::cerr << "Error: Palette file does not exist: " << config.palette_file
              << std::endl;
    return 1;
  }

  // Print banner if verbose
  if (config.verbose && !config.quiet) {
    std::cout << "art2img v1.0.0 - Duke Nukem 3D ART File Converter"
              << std::endl;
    std::cout << "===============================================" << std::endl;
    std::cout << std::endl;
  }

  // Process input
  int result;
  if (std::filesystem::is_directory(config.input_path)) {
    if (config.export_animation) {
      std::cerr << "Error: Animation export is not supported for directories"
                << std::endl;
      return 1;
    }
    result = process_art_directory(config);
  } else {
    if (config.export_animation) {
      result = process_animation_export(config);

      if (!config.quiet) {
        std::cout << "Animation export completed successfully" << std::endl;
      }
    } else {
      ProgressTracker progress;
      result = process_art_file(config, progress);

      if (!config.quiet) {
        std::cout
            << std::format(
                   "Conversion complete: {} tiles processed, {} tiles failed",
                   progress.completed, progress.failed)
            << std::endl;
      }
    }
  }

  return result;
}
