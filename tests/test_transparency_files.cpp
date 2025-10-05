#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "exceptions.hpp"
#include "extractor_api.hpp"
#include "image_writer.hpp"
#include "palette.hpp"
#include "test_helpers.hpp"

// Test transparency functionality with actual file output
TEST_CASE("Transparency - File output validation") {
  // Skip test if required assets are not available
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping transparency file tests");
    return;
  }

  // Create unique test directory
  std::string timestamp = std::to_string(std::time(nullptr));
  std::string test_dir = "build/tests/transparency_file_" + timestamp;
  std::filesystem::create_directories(test_dir);

  // Load test assets
  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");
  REQUIRE_FALSE(art_data.empty());
  REQUIRE_GE(palette_data.size(), art2img::Palette::SIZE);

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("With fix_transparency enabled - PNG output") {
    std::string output_dir = test_dir + "/fix_enabled_png";
    std::filesystem::create_directories(output_dir);

    // Extract first few tiles with transparency enabled
    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    // Extract 5 tiles to files
    for (int i = 0; i < 5 && i < static_cast<int>(extractor.get_tile_count()); ++i) {
      std::string filename = output_dir + "/tile" + std::to_string(i) + ".png";
      auto result = extractor.extract_tile(i, art2img::ImageFormat::PNG, options);
      REQUIRE(result.success);

      if (!result.image_data.empty()) {
        // Write to file
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();
        CHECK(std::filesystem::exists(filename));
        CHECK_GT(std::filesystem::file_size(filename), 0);
      }
    }

    // Verify output directory has files
    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".png") {
        file_count++;
      }
    }
    CHECK_GT(file_count, 0);
  }

  SUBCASE("With fix_transparency disabled - PNG output") {
    std::string output_dir = test_dir + "/fix_disabled_png";
    std::filesystem::create_directories(output_dir);

    // Extract first few tiles with transparency disabled
    art2img::ImageWriter::Options options;
    options.fix_transparency = false;
    options.enable_alpha = true;

    // Extract 5 tiles to files
    for (int i = 0; i < 5 && i < static_cast<int>(extractor.get_tile_count()); ++i) {
      std::string filename = output_dir + "/tile" + std::to_string(i) + ".png";
      auto result = extractor.extract_tile(i, art2img::ImageFormat::PNG, options);
      REQUIRE(result.success);

      if (!result.image_data.empty()) {
        // Write to file
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();
        CHECK(std::filesystem::exists(filename));
        CHECK_GT(std::filesystem::file_size(filename), 0);
      }
    }

    // Verify output directory has files
    int file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(output_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".png") {
        file_count++;
      }
    }
    CHECK_GT(file_count, 0);
  }

  SUBCASE("Format comparison - same tile, different formats") {
    std::string output_dir = test_dir + "/format_comparison";
    std::filesystem::create_directories(output_dir);

    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    int test_tile_index = 0;  // Use first tile for comparison

    // Extract as PNG
    {
      std::string filename = output_dir + "/tile_png.png";
      auto result = extractor.extract_tile(test_tile_index, art2img::ImageFormat::PNG, options);
      REQUIRE(result.success);

      if (!result.image_data.empty()) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();
        CHECK(std::filesystem::exists(filename));
      }
    }

    // Extract as TGA
    {
      std::string filename = output_dir + "/tile_tga.tga";
      auto result = extractor.extract_tile(test_tile_index, art2img::ImageFormat::TGA, options);
      REQUIRE(result.success);

      if (!result.image_data.empty()) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();
        CHECK(std::filesystem::exists(filename));
      }
    }

    // Extract as BMP
    {
      std::string filename = output_dir + "/tile_bmp.bmp";
      auto result = extractor.extract_tile(test_tile_index, art2img::ImageFormat::BMP, options);
      REQUIRE(result.success);

      if (!result.image_data.empty()) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();
        CHECK(std::filesystem::exists(filename));
      }
    }

    // Verify all formats were created
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
    std::filesystem::remove_all(test_dir);
  }
}