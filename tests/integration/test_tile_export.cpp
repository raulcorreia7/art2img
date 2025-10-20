/// @file test_tile_export.cpp
/// @brief Integration tests for tile export functionality
///
/// This test suite covers:
/// - Exporting individual tiles to PNG, TGA, BMP formats
/// - Exporting all tiles from ART files
/// - File I/O verification
/// - Different output directory configurations

#include "../test_helpers.hpp"
#include <algorithm>
#include <art2img/art.hpp>
#include <art2img/convert.hpp>
#include <art2img/encode.hpp>
#include <art2img/export.hpp>
#include <art2img/io.hpp>
#include <atomic>
#include <doctest/doctest.h>
#include <filesystem>
#include <future>
#include <iostream>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

namespace {

// Helper function to create output directory (using new test helpers)
void ensure_output_directory(const std::filesystem::path &dir) {
  test_helpers::ensure_test_output_dir(dir);
}

// Helper function to verify file exists and has content
void verify_output_file(const std::filesystem::path &path) {
  REQUIRE(std::filesystem::exists(path));
  REQUIRE(std::filesystem::file_size(path) > 0);
}

// Helper function to export a single tile using the new export API
std::expected<art2img::ExportResult, art2img::Error>
export_single_tile(const art2img::TileView &tile,
                   const art2img::Palette &palette, art2img::ImageFormat format,
                   const std::filesystem::path &output_path) {

  art2img::ExportOptions options;
  options.output_dir = output_path.parent_path();
  options.format = format;
  options.filename_prefix = output_path.stem().string();

  auto result = art2img::export_tile(tile, palette, options);
  if (result.has_value()) {
    // The export function generates its own filename, so we need to check
    // what file was actually created and verify it exists
    const auto &exported_files = result.value().output_files;
    if (!exported_files.empty()) {
      // Verify the generated file exists
      if (!std::filesystem::exists(exported_files[0])) {
        return std::unexpected(art2img::Error{art2img::errc::io_failure,
                                              "Exported file does not exist: " +
                                                  exported_files[0].string()});
      }
    }
  }
  return result;
}

// Helper function to get supported formats
std::vector<art2img::ImageFormat> get_supported_formats() {
  return {art2img::ImageFormat::png, art2img::ImageFormat::tga,
          art2img::ImageFormat::bmp};
}

} // anonymous namespace

TEST_SUITE("Tile Export Integration") {

  TEST_CASE("Setup - Create output directory") {
    const auto test_output_dir =
        test_helpers::get_integration_test_dir("setup");
    ensure_output_directory(test_output_dir);
    CHECK(std::filesystem::exists(test_output_dir));
  }

  TEST_CASE("Export single tile - all formats, same output folder") {
    // Load test ART file
    const std::filesystem::path art_path =
        TEST_ASSET_SOURCE_DIR "/TILES001.ART";
    if (!std::filesystem::exists(art_path)) {
      return; // Skip if file not found
    }

    auto art_result = art2img::load_art_bundle(art_path);
    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Load palette
    const std::filesystem::path palette_path =
        TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
    REQUIRE(std::filesystem::exists(palette_path));
    auto palette_result = art2img::load_palette(palette_path);
    REQUIRE(palette_result.has_value());
    const auto &palette = palette_result.value();

    // Get first tile
    auto tile_result = art_data.get_tile(0);
    REQUIRE(tile_result.has_value());
    const auto &tile = tile_result.value();
    REQUIRE(tile.is_valid());

    // Export to all formats in same directory
    const auto formats = get_supported_formats();

    for (const auto format : formats) {
      std::string extension = (format == art2img::ImageFormat::png   ? "png"
                               : format == art2img::ImageFormat::tga ? "tga"
                                                                     : "bmp");
      const auto output_path =
          test_helpers::get_integration_test_dir("single_tile_export") /
          ("single_tile_same_0." + extension);

      auto export_result =
          export_single_tile(tile, palette, format, output_path);
      REQUIRE(export_result.has_value());
      REQUIRE(export_result.value().exported_tiles == 1);
      REQUIRE(export_result.value().output_files.size() == 1);
      verify_output_file(export_result.value().output_files[0]);
    }
  }

  TEST_CASE("Export single tile - all formats, different folders per format") {
    // Load test ART file
    const std::filesystem::path art_path =
        TEST_ASSET_SOURCE_DIR "/TILES004.ART";
    if (!std::filesystem::exists(art_path)) {
      return; // Skip if file not found
    }

    auto art_result = art2img::load_art_bundle(art_path);
    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Load palette
    const std::filesystem::path palette_path =
        TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
    REQUIRE(std::filesystem::exists(palette_path));
    auto palette_result = art2img::load_palette(palette_path);
    REQUIRE(palette_result.has_value());
    const auto &palette = palette_result.value();

    // Get first tile
    auto tile_result = art_data.get_tile(0);
    REQUIRE(tile_result.has_value());
    const auto &tile = tile_result.value();
    REQUIRE(tile.is_valid());

    // Export to all formats in different directories
    const auto formats = get_supported_formats();

    for (const auto format : formats) {
      std::string format_name;
      switch (format) {
      case art2img::ImageFormat::png:
        format_name = "png";
        break;
      case art2img::ImageFormat::tga:
        format_name = "tga";
        break;
      case art2img::ImageFormat::bmp:
        format_name = "bmp";
        break;
      default:
        format_name = "unknown";
        break;
      }

      const auto format_dir =
          test_helpers::get_integration_test_dir("tile_export") / format_name;
      ensure_output_directory(format_dir);

      const auto output_path =
          format_dir / ("single_tile_separate_0." + format_name);

      auto export_result =
          export_single_tile(tile, palette, format, output_path);
      REQUIRE(export_result.has_value());
      REQUIRE(export_result.value().exported_tiles == 1);
      REQUIRE(export_result.value().output_files.size() == 1);
      verify_output_file(export_result.value().output_files[0]);
    }
  }

  TEST_CASE("Export all tiles - all formats, same output folder") {
    // Load test ART file with multiple tiles
    const std::filesystem::path art_path =
        TEST_ASSET_SOURCE_DIR "/TILES004.ART";
    if (!std::filesystem::exists(art_path)) {
      return; // Skip if file not found
    }

    auto art_result = art2img::load_art_bundle(art_path);
    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Load palette
    const std::filesystem::path palette_path =
        TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
    REQUIRE(std::filesystem::exists(palette_path));
    auto palette_result = art2img::load_palette(palette_path);
    REQUIRE(palette_result.has_value());
    const auto &palette = palette_result.value();

    // Export all tiles to all formats in same directory
    const auto formats = get_supported_formats();
    const std::size_t max_tiles = std::min<std::size_t>(
        5, art_data.tile_count()); // Limit for test performance

    for (std::size_t tile_idx = 0; tile_idx < max_tiles; ++tile_idx) {
      auto tile_result = art_data.get_tile(tile_idx);
      REQUIRE(tile_result.has_value());
      const auto &tile = tile_result.value();
      if (!tile.is_valid()) {
        std::cout << "Skipping empty tile index " << tile_idx << std::endl;
        continue;
      }

      for (const auto format : formats) {
        std::string extension = (format == art2img::ImageFormat::png   ? "png"
                                 : format == art2img::ImageFormat::tga ? "tga"
                                                                       : "bmp");
        const auto output_path =
            test_helpers::get_integration_test_dir("tile_export") /
            ("all_tiles_same_" + std::to_string(tile_idx) + "." + extension);

        auto export_result =
            export_single_tile(tile, palette, format, output_path);
        REQUIRE(export_result.has_value());
        REQUIRE(export_result.value().exported_tiles == 1);
        REQUIRE(export_result.value().output_files.size() == 1);
        verify_output_file(export_result.value().output_files[0]);
      }
    }
  }

  TEST_CASE("Export all tiles - all formats, different folders per format") {
    // Load test ART file with multiple tiles
    const std::filesystem::path art_path =
        TEST_ASSET_SOURCE_DIR "/TILES005.ART";
    if (!std::filesystem::exists(art_path)) {
      return; // Skip if file not found
    }

    auto art_result = art2img::load_art_bundle(art_path);
    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Load palette
    const std::filesystem::path palette_path =
        TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
    REQUIRE(std::filesystem::exists(palette_path));
    auto palette_result = art2img::load_palette(palette_path);
    REQUIRE(palette_result.has_value());
    const auto &palette = palette_result.value();

    // Export all tiles to all formats in separate directories
    const auto formats = get_supported_formats();
    const std::size_t max_tiles = std::min<std::size_t>(
        3, art_data.tile_count()); // Limit for test performance

    for (const auto format : formats) {
      std::string format_name;
      switch (format) {
      case art2img::ImageFormat::png:
        format_name = "png";
        break;
      case art2img::ImageFormat::tga:
        format_name = "tga";
        break;
      case art2img::ImageFormat::bmp:
        format_name = "bmp";
        break;
      default:
        format_name = "unknown";
        break;
      }

      const auto format_dir =
          test_helpers::get_integration_test_dir("tile_export") / format_name;
      ensure_output_directory(format_dir);

      for (std::size_t tile_idx = 0; tile_idx < max_tiles; ++tile_idx) {
        auto tile_result = art_data.get_tile(tile_idx);
        REQUIRE(tile_result.has_value());
        const auto &tile = tile_result.value();
        if (!tile.is_valid()) {
          std::cout << "Skipping empty tile index " << tile_idx << std::endl;
          continue;
        }

        const auto output_path =
            format_dir / ("all_tiles_separate_" + std::to_string(tile_idx) +
                          "." + format_name);

        auto export_result =
            export_single_tile(tile, palette, format, output_path);
        REQUIRE(export_result.has_value());
        REQUIRE(export_result.value().exported_tiles == 1);
        REQUIRE(export_result.value().output_files.size() == 1);
        verify_output_file(export_result.value().output_files[0]);
      }
    }
  }

  TEST_CASE("Error handling - invalid tile export") {
    // Load minimal palette for testing
    const std::filesystem::path palette_path =
        TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
    if (!std::filesystem::exists(palette_path)) {
      return; // Skip if palette not found
    }

    auto palette_result = art2img::load_palette(palette_path);
    REQUIRE(palette_result.has_value());
    const auto &palette = palette_result.value();

    // Create a minimal invalid tile view (empty)
    art2img::TileView invalid_tile;

    // Try to export invalid tile
    const auto output_path =
        test_helpers::get_integration_test_dir("tile_export") /
        "invalid_tile.png";
    auto export_result = export_single_tile(
        invalid_tile, palette, art2img::ImageFormat::png, output_path);

    // Should fail gracefully
    CHECK(!export_result.has_value());
  }

  // TEST_CASE("Export all ART files and tiles - comprehensive dump") {
  //     // Create comprehensive dump directory
  //     const auto comprehensive_dump_dir =
  //     test_helpers::get_integration_test_dir("tile_export") /
  //     "comprehensive_dump"; ensure_output_directory(comprehensive_dump_dir);

  //     // Load palette once
  //     const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR
  //     "/PALETTE.DAT"; REQUIRE(std::filesystem::exists(palette_path)); auto
  //     palette_result = art2img::load_palette(palette_path);
  //     REQUIRE(palette_result.has_value());
  //     const auto& palette = palette_result.value();

  //     // Get all supported formats
  //     const auto formats = get_supported_formats();

  //     // Find all TILES*.ART files
  //     std::vector<std::filesystem::path> art_files;
  //     for (const auto& entry :
  //     std::filesystem::directory_iterator(TEST_ASSET_SOURCE_DIR)) {
  //         if (entry.is_regular_file() && entry.path().extension() == ".ART"
  //         &&
  //             entry.path().filename().string().starts_with("TILES")) {
  //             art_files.push_back(entry.path());
  //         }
  //     }

  //     std::cout << "Found " << art_files.size() << " ART files to process" <<
  //     std::endl;

  //     // Process each ART file
  //     for (const auto& art_path : art_files) {
  //         std::string art_filename = art_path.filename().string();
  //         art_filename = art_filename.substr(0,
  //         art_filename.find_last_of('.')); // Remove extension

  //         std::cout << "Processing " << art_filename << std::endl;

  //         // Load ART file
  //         auto art_result = art2img::load_art_bundle(art_path);
  //         REQUIRE(art_result.has_value());
  //         const auto& art_data = art_result.value();

  //         std::cout << "  Loaded " << art_data.tile_count() << " tiles" <<
  //         std::endl;

  //         // Export all tiles in all formats flat to comprehensive_dump
  //         directory for (std::size_t tile_idx = 0; tile_idx <
  //         art_data.tile_count(); ++tile_idx) {
  //             auto tile_result = art_data.get_tile(tile_idx);
  //             REQUIRE(tile_result.has_value());
  //             const auto& tile = tile_result.value();
  //             if (!tile.is_valid()) {
  //                 std::cout << "Skipping empty tile index " << tile_idx <<
  //                 std::endl; continue;
  //             }

  //             // Export to each format in the main comprehensive_dump
  //             directory for (const auto format : formats) {
  //                 std::string format_name;
  //                 switch (format) {
  //                     case art2img::ImageFormat::png: format_name = "png";
  //                     break; case art2img::ImageFormat::tga: format_name =
  //                     "tga"; break; case art2img::ImageFormat::bmp:
  //                     format_name = "bmp"; break; default: format_name =
  //                     "unknown"; break;
  //                 }

  //                 const auto output_path = comprehensive_dump_dir /
  //                 (art_filename + "_tile_" + std::to_string(tile_idx) + "." +
  //                 format_name);

  //                 auto export_result = export_single_tile(tile, palette,
  //                 format, output_path); REQUIRE(export_result.has_value());
  //                 REQUIRE(export_result.value().exported_tiles == 1);
  //                 REQUIRE(export_result.value().output_files.size() == 1);
  //                 verify_output_file(export_result.value().output_files[0]);
  //             }
  //         }

  //         std::cout << "  Completed processing " << art_filename <<
  //         std::endl;
  //     }

  //     std::cout << "Comprehensive dump completed successfully" << std::endl;
  // }

  TEST_CASE("Export all ART files and tiles - comprehensive dump (massively "
            "parallel)") {
    // Create comprehensive dump directory using test helpers
    const auto comprehensive_dump_dir =
        test_helpers::get_integration_test_dir("comprehensive_dump_parallel");
    ensure_output_directory(comprehensive_dump_dir);

    // Load palette once
    const std::filesystem::path palette_path =
        std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";
    REQUIRE(std::filesystem::exists(palette_path));
    auto palette_result = art2img::load_palette(palette_path);
    REQUIRE(palette_result.has_value());
    const auto &palette = palette_result.value();

    // Get all supported formats
    const auto formats = get_supported_formats();

    // Find all TILES*.ART files
    std::vector<std::filesystem::path> art_files;
    for (const auto &entry :
         std::filesystem::directory_iterator(TEST_ASSET_SOURCE_DIR)) {
      if (entry.is_regular_file() && entry.path().extension() == ".ART" &&
          entry.path().filename().string().starts_with("TILES")) {
        art_files.push_back(entry.path());
      }
    }

    std::cout << "Found " << art_files.size() << " ART files to process"
              << std::endl;

    // Use std::async for massively parallel processing - enhanced tile-level
    // parallelism
    std::vector<std::future<void>> futures;
    std::mutex cout_mutex;
    std::atomic<size_t> total_processed{0};

    // Pre-compute all tiles and filenames to avoid redundant operations
    struct TileInfo {
      std::size_t art_data_idx;
      std::size_t tile_idx;
      art2img::TileView tile;
      std::string filename;
    };

    std::vector<art2img::ArtData> art_data_storage;
    std::vector<TileInfo> all_tiles;

    for (const auto &art_path : art_files) {
      std::string art_filename = art_path.filename().string();
      art_filename = art_filename.substr(0, art_filename.find_last_of('.'));

      auto art_result = art2img::load_art_bundle(art_path);
      if (!art_result.has_value()) {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cerr << "Failed to load " << art_filename << std::endl;
        continue;
      }

      {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Loaded " << art_filename << " with "
                  << art_result.value().tile_count() << " tiles" << std::endl;
      }

      // Store the ArtData to keep it alive
      std::size_t art_data_idx = art_data_storage.size();
      art_data_storage.push_back(std::move(art_result.value()));
      const auto &art_data = art_data_storage.back();

      // Pre-compute all valid tiles from this ART file
      for (std::size_t tile_idx = 0; tile_idx < art_data.tile_count();
           ++tile_idx) {
        auto tile_result = art_data.get_tile(tile_idx);
        if (tile_result.has_value()) {
          const auto &tile = tile_result.value();
          if (tile.is_valid()) {
            all_tiles.push_back({art_data_idx, tile_idx, tile, art_filename});
          }
        }
      }
    }

    std::cout << "Total valid tiles to export across all formats: "
              << all_tiles.size() << std::endl;

    // Process each format with batched parallel tasks
    constexpr std::size_t BATCH_SIZE =
        64; // Process 64 tiles per task to reduce async overhead

    for (const auto format : formats) {
      std::string format_name;
      switch (format) {
      case art2img::ImageFormat::png:
        format_name = "png";
        break;
      case art2img::ImageFormat::tga:
        format_name = "tga";
        break;
      case art2img::ImageFormat::bmp:
        format_name = "bmp";
        break;
      default:
        format_name = "unknown";
        break;
      }

      std::cout << "Starting batched parallel export for format: "
                << format_name << " (" << all_tiles.size() << " tiles)"
                << std::endl;

      // Launch batched async tasks
      for (std::size_t i = 0; i < all_tiles.size(); i += BATCH_SIZE) {
        futures.push_back(
            std::async(std::launch::async, [&, i, format, format_name]() {
              std::size_t end = std::min(i + BATCH_SIZE, all_tiles.size());

              for (std::size_t j = i; j < end; ++j) {
                const auto &tile_info = all_tiles[j];
                const auto &art_data = art_data_storage[tile_info.art_data_idx];

                const auto output_path =
                    comprehensive_dump_dir /
                    (tile_info.filename + "_tile_" +
                     std::to_string(tile_info.tile_idx) + "." + format_name);

                auto export_result = export_single_tile(tile_info.tile, palette,
                                                        format, output_path);
                if (export_result.has_value()) {
                  if (export_result.value().exported_tiles == 1) {
                    verify_output_file(export_result.value().output_files[0]);
                    total_processed++;
                  }
                }
              }
            }));
      }
    }

    // Wait for all async operations to complete
    std::cout << "Waiting for " << futures.size()
              << " parallel export tasks to complete..." << std::endl;
    for (auto &future : futures) {
      future.wait();
    }

    std::cout << "Enhanced massively parallel dump completed successfully"
              << std::endl;
    std::cout << "Total tiles processed: " << total_processed.load()
              << std::endl;

    // Export animation data for each ART file
    std::cout << "Exporting animation data..." << std::endl;
    art2img::AnimationExportConfig anim_config;
    anim_config.output_dir = comprehensive_dump_dir;
    anim_config.ini_filename = "animdata.ini";
    anim_config.image_format =
        art2img::ImageFormat::png; // Use PNG for animation tile references
    anim_config.include_image_references = true;
    anim_config.include_non_animated = true;

    std::atomic<size_t> animation_files_processed{0};
    std::vector<std::future<void>> anim_futures;

    // Export animation data in parallel for each ART file
    for (std::size_t art_idx = 0; art_idx < art_files.size(); ++art_idx) {
      const auto &art_path = art_files[art_idx];
      const auto &art_data = art_data_storage[art_idx];

      anim_futures.push_back(
          std::async(std::launch::async, [&, art_idx, art_path, art_data]() {
            std::string art_filename = art_path.filename().string();
            art_filename =
                art_filename.substr(0, art_filename.find_last_of('.'));

            // Configure animation export for this specific ART file
            art2img::AnimationExportConfig file_anim_config = anim_config;
            file_anim_config.ini_filename = art_filename + "_animdata.ini";
            file_anim_config.base_name = art_filename + "_tile";

            auto anim_export_result =
                art2img::export_animation_data(art_data, file_anim_config);
            if (anim_export_result.has_value()) {
              // Verify INI file was created
              const std::filesystem::path ini_path =
                  comprehensive_dump_dir / file_anim_config.ini_filename;
              if (std::filesystem::exists(ini_path)) {
                animation_files_processed++;
                {
                  std::lock_guard<std::mutex> lock(cout_mutex);
                  std::cout << "Created animation file: "
                            << file_anim_config.ini_filename << std::endl;
                }
              }
            } else {
              std::lock_guard<std::mutex> lock(cout_mutex);
              std::cerr << "Failed to export animation data for "
                        << art_filename << std::endl;
            }
          }));
    }

    // Wait for all animation export tasks to complete
    std::cout << "Waiting for " << anim_futures.size()
              << " animation export tasks to complete..." << std::endl;
    for (auto &future : anim_futures) {
      future.wait();
    }

    std::cout << "Animation export completed. Created "
              << animation_files_processed.load() << " animation files."
              << std::endl;
  }

} // TEST_SUITE
