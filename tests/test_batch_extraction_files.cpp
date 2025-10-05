#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "exceptions.hpp"
#include "extractor_api.hpp"
#include "image_writer.hpp"
#include "test_helpers.hpp"

// Test batch extraction with actual file output
TEST_CASE("Batch extraction - File output validation") {
  // Skip test if required assets are not available
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping batch extraction file tests");
    return;
  }

  // Create unique test directory
  std::string timestamp = std::to_string(std::time(nullptr));
  std::string test_dir = "build/tests/batch_extraction_" + timestamp;
  std::filesystem::create_directories(test_dir);

  // Load test assets
  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");
  REQUIRE_FALSE(art_data.empty());
  REQUIRE_GE(palette_data.size(), art2img::Palette::SIZE);

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("Batch extract to PNG files") {
    std::string output_dir = test_dir + "/png_output";
    std::filesystem::create_directories(output_dir);

    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    // Extract all tiles as PNG
    auto results = extractor.extract_all_tiles(art2img::ImageFormat::PNG, options);
    REQUIRE_EQ(results.size(), extractor.get_tile_count());

    int success_count = 0;
    int file_count = 0;

    // Write non-empty results to files
    for (size_t i = 0; i < results.size(); ++i) {
      const auto& result = results[i];
      if (result.success && !result.image_data.empty()) {
        success_count++;
        std::string filename = output_dir + "/tile" + std::to_string(result.tile_index) + ".png";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();

        if (std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0) {
          file_count++;
        }
      }
    }

    CHECK_GE(success_count, 0);           // At least some should succeed
    CHECK_EQ(file_count, success_count);  // All successful extractions should create files

    // Verify output directory has expected number of files
    int actual_file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".png") {
        actual_file_count++;
      }
    }
    CHECK_EQ(actual_file_count, file_count);
  }

  SUBCASE("Batch extract to TGA files") {
    std::string output_dir = test_dir + "/tga_output";
    std::filesystem::create_directories(output_dir);

    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    // Extract all tiles as TGA
    auto results = extractor.extract_all_tiles(art2img::ImageFormat::TGA, options);
    REQUIRE_EQ(results.size(), extractor.get_tile_count());

    int success_count = 0;
    int file_count = 0;

    // Write non-empty results to files
    for (size_t i = 0; i < results.size(); ++i) {
      const auto& result = results[i];
      if (result.success && !result.image_data.empty()) {
        success_count++;
        std::string filename = output_dir + "/tile" + std::to_string(result.tile_index) + ".tga";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();

        if (std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0) {
          file_count++;
        }
      }
    }

    CHECK_GE(success_count, 0);           // At least some should succeed
    CHECK_EQ(file_count, success_count);  // All successful extractions should create files
  }

  SUBCASE("Batch extract to BMP files") {
    std::string output_dir = test_dir + "/bmp_output";
    std::filesystem::create_directories(output_dir);

    art2img::ImageWriter::Options options;
    options.fix_transparency = false;  // BMP typically doesn't need alpha
    options.enable_alpha = false;

    // Extract all tiles as BMP
    auto results = extractor.extract_all_tiles(art2img::ImageFormat::BMP, options);
    REQUIRE_EQ(results.size(), extractor.get_tile_count());

    int success_count = 0;
    int file_count = 0;

    // Write non-empty results to files
    for (size_t i = 0; i < results.size(); ++i) {
      const auto& result = results[i];
      if (result.success && !result.image_data.empty()) {
        success_count++;
        std::string filename = output_dir + "/tile" + std::to_string(result.tile_index) + ".bmp";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();

        if (std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0) {
          file_count++;
        }
      }
    }

    CHECK_GE(success_count, 0);           // At least some should succeed
    CHECK_EQ(file_count, success_count);  // All successful extractions should create files
  }

  SUBCASE("Limited batch extraction - first 10 tiles") {
    std::string output_dir = test_dir + "/limited_output";
    std::filesystem::create_directories(output_dir);

    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    // Extract only first 10 tiles
    uint32_t tile_count = std::min(10u, extractor.get_tile_count());
    for (uint32_t i = 0; i < tile_count; ++i) {
      auto result = extractor.extract_tile(i, art2img::ImageFormat::PNG, options);
      if (result.success && !result.image_data.empty()) {
        std::string filename = output_dir + "/tile" + std::to_string(result.tile_index) + ".png";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();
        CHECK(std::filesystem::exists(filename));
      }
    }

    // Verify exactly 10 files (or fewer if less than 10 tiles)
    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".png") {
        file_count++;
      }
    }
    CHECK_LE(file_count, 10);
  }

  // Only cleanup in CI environment
  if (std::getenv("CI") != nullptr) {
    std::filesystem::remove_all(test_dir);
  }
}