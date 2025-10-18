#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "art2img/exceptions.hpp"
#include "art2img/extractor_api.hpp"
#include "art2img/image_writer.hpp"
#include "test_helpers.hpp"

// Test output directory handling
TEST_CASE("Output directory handling") {
  // Skip test if required assets are not available
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping output directory tests");
    return;
  }

  // Create unique test directory
  std::string timestamp = std::to_string(std::time(nullptr));
  std::string base_test_dir = "build/tests/output_dirs_" + timestamp;
  std::filesystem::create_directories(base_test_dir);

  // Load test assets
  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");
  REQUIRE_FALSE(art_data.empty());
  REQUIRE_GE(palette_data.size(), art2img::Palette::SIZE);

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("Create new output directory") {
    std::string output_dir = base_test_dir + "/new_directory/test/subdir";

    // Directory should not exist yet
    CHECK_FALSE(std::filesystem::exists(output_dir));

    // Extract a tile to this directory (should create it)
    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    auto result = extractor.extract_tile(0, art2img::ImageFormat::PNG, options);
    if (result.success && !result.image_data.empty()) {
      std::string filename = output_dir + "/test_tile.png";
      std::filesystem::create_directories(output_dir);  // Ensure directory exists
      std::ofstream file(filename, std::ios::binary);
      file.write(reinterpret_cast<const char*>(result.image_data.data()), result.image_data.size());
      file.close();

      CHECK(std::filesystem::exists(output_dir));
      CHECK(std::filesystem::exists(filename));
      CHECK_GT(std::filesystem::file_size(filename), 0);
    }
  }

  SUBCASE("Use existing output directory") {
    std::string output_dir = base_test_dir + "/existing_directory";
    std::filesystem::create_directories(output_dir);

    // Directory should exist
    CHECK(std::filesystem::exists(output_dir));

    // Extract multiple tiles to this directory
    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    int files_created = 0;
    for (int i = 0; i < 3 && i < static_cast<int>(extractor.get_tile_count()); ++i) {
      auto result = extractor.extract_tile(i, art2img::ImageFormat::PNG, options);
      if (result.success && !result.image_data.empty()) {
        std::string filename = output_dir + "/tile" + std::to_string(i) + ".png";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();

        if (std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0) {
          files_created++;
        }
      }
    }

    CHECK_EQ(files_created, 3);

    // Verify directory still exists and has files
    CHECK(std::filesystem::exists(output_dir));
    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".png") {
        file_count++;
      }
    }
    CHECK_EQ(file_count, files_created);
  }

  SUBCASE("Nested directory structure") {
    std::string output_dir = base_test_dir + "/nested/level1/level2/level3";

    // Extract tile to deeply nested directory
    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    auto result = extractor.extract_tile(0, art2img::ImageFormat::PNG, options);
    if (result.success && !result.image_data.empty()) {
      std::string filename = output_dir + "/nested_tile.png";
      std::filesystem::create_directories(output_dir);  // Ensure all parent directories exist
      std::ofstream file(filename, std::ios::binary);
      file.write(reinterpret_cast<const char*>(result.image_data.data()), result.image_data.size());
      file.close();

      // Verify full path exists
      CHECK(std::filesystem::exists(output_dir));
      CHECK(std::filesystem::exists(filename));
      CHECK_GT(std::filesystem::file_size(filename), 0);
    }
  }

  SUBCASE("Multiple formats in same directory") {
    std::string output_dir = base_test_dir + "/mixed_formats";
    std::filesystem::create_directories(output_dir);

    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    int tile_index = 0;  // Use first tile

    // Extract same tile in different formats to same directory
    std::vector<std::pair<art2img::ImageFormat, std::string>> formats = {
        {art2img::ImageFormat::PNG, "test.png"},
        {art2img::ImageFormat::TGA, "test.tga"},
        {art2img::ImageFormat::BMP, "test.bmp"}};

    int files_created = 0;
    for (const auto& [format, filename] : formats) {
      auto result = extractor.extract_tile(tile_index, format, options);
      if (result.success && !result.image_data.empty()) {
        std::string full_path = output_dir + "/" + filename;
        std::ofstream file(full_path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();

        if (std::filesystem::exists(full_path) && std::filesystem::file_size(full_path) > 0) {
          files_created++;
        }
      }
    }

    CHECK_EQ(files_created, 3);

    // Verify all three files exist
    int png_count = 0, tga_count = 0, bmp_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
      if (entry.is_regular_file()) {
        auto ext = entry.path().extension();
        if (ext == ".png")
          png_count++;
        else if (ext == ".tga")
          tga_count++;
        else if (ext == ".bmp")
          bmp_count++;
      }
    }

    CHECK_EQ(png_count, 1);
    CHECK_EQ(tga_count, 1);
    CHECK_EQ(bmp_count, 1);
  }

  // Only cleanup in CI environment
  if (std::getenv("CI") != nullptr) {
    std::filesystem::remove_all(base_test_dir);
  }
}