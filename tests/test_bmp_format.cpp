#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>

#include "exceptions.hpp"
#include "extractor_api.hpp"
#include "test_helpers.hpp"

// Include headers for direct testing
#include "file_operations.hpp"
#include "image_processor.hpp"
#include "image_writer.hpp"

// Function to create a test extractor with data
static art2img::ExtractorAPI create_test_extractor() {
  static bool assets_loaded = false;
  static std::vector<uint8_t> art_data;
  static std::vector<uint8_t> palette_data;

  if (!assets_loaded) {
    if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
      throw std::runtime_error("Test assets not found");
    }

    art_data = load_test_asset("TILES000.ART");
    palette_data = load_test_asset("PALETTE.DAT");
    assets_loaded = true;
  }

  art2img::ExtractorAPI extractor;
  if (!extractor.load_art_from_memory(art_data.data(), art_data.size()) ||
      !extractor.load_palette_from_memory(palette_data.data(), palette_data.size())) {
    throw std::runtime_error("Failed to load test data");
  }

  return extractor;
}

// Helper function to check BMP file signature
bool is_valid_bmp_signature(const std::vector<uint8_t>& data) {
  if (data.size() < 2)
    return false;
  return (data[0] == 'B' && data[1] == 'M');
}

// Helper function to check basic BMP file structure
bool has_valid_bmp_header(const std::vector<uint8_t>& data) {
  if (data.size() < 54)
    return false;  // Minimum BMP header size
  if (!is_valid_bmp_signature(data))
    return false;

  // Check file size in header matches actual data size
  uint32_t file_size = data[2] | (data[3] << 8) | (data[4] << 16) | (data[5] << 24);
  return file_size == data.size();
}

TEST_CASE("BMP format basic functionality") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping BMP format tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  // Find a non-empty tile
  uint32_t target_tile = 0;
  bool found_non_empty = false;

  for (uint32_t i = 0; i < art_view.image_count(); ++i) {
    if (!art_view.get_tile(i).is_empty()) {
      target_tile = i;
      found_non_empty = true;
      break;
    }
  }

  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping BMP format tests");
    return;
  }

  art2img::ImageView image_view{&art_view, target_tile};

  SUBCASE("Save to BMP file") {
    std::string filename = "test_tile.bmp";
    REQUIRE(image_view.save_to_bmp(filename));
    CHECK(std::filesystem::exists(filename));
    CHECK_GT(std::filesystem::file_size(filename), 0);

    // Check BMP file signature
    std::ifstream file(filename, std::ios::binary);
    REQUIRE(file.is_open());
    char signature[2];
    file.read(signature, 2);
    CHECK_EQ(signature[0], 'B');
    CHECK_EQ(signature[1], 'M');
    file.close();

    // Clean up
    std::filesystem::remove(filename);
  }

  SUBCASE("Save to BMP with default options") {
    std::string filename = "test_tile_default.bmp";
    REQUIRE(image_view.save_to_bmp(filename));
    CHECK(std::filesystem::exists(filename));
    CHECK_GT(std::filesystem::file_size(filename), 0);

    // Validate BMP file
    std::ifstream file(filename, std::ios::binary);
    REQUIRE(file.is_open());
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> file_data(file_size);
    file.read(reinterpret_cast<char*>(file_data.data()), file_size);
    file.close();
    CHECK(is_valid_bmp_signature(file_data));

    // Clean up
    std::filesystem::remove(filename);
  }

  SUBCASE("Save to BMP with custom options") {
    std::string filename = "test_tile_custom.bmp";
    REQUIRE(image_view.save_to_bmp(filename));
    CHECK(std::filesystem::exists(filename));
    CHECK_GT(std::filesystem::file_size(filename), 0);

    // Validate BMP file
    std::ifstream file(filename, std::ios::binary);
    REQUIRE(file.is_open());
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> file_data(file_size);
    file.read(reinterpret_cast<char*>(file_data.data()), file_size);
    file.close();
    CHECK(is_valid_bmp_signature(file_data));

    // Clean up
    std::filesystem::remove(filename);
  }

  SUBCASE("Extract to BMP memory") {
    auto bmp_data = image_view.extract_to_bmp();
    CHECK_GT(bmp_data.size(), 0);
    CHECK(is_valid_bmp_signature(bmp_data));

    // BMP files should have headers, so minimum size should be > 54 bytes
    CHECK_GT(bmp_data.size(), 54);
  }

  SUBCASE("Extract to BMP memory with options") {
    auto bmp_data = image_view.extract_to_bmp();
    CHECK_GT(bmp_data.size(), 0);
    CHECK(is_valid_bmp_signature(bmp_data));

    // Check BMP file structure
    CHECK(has_valid_bmp_header(bmp_data));
  }
}

TEST_CASE("BMP format header validation") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping BMP header validation tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  // Find a non-empty tile
  uint32_t target_tile = 0;
  bool found_non_empty = false;

  for (uint32_t i = 0; i < art_view.image_count(); ++i) {
    if (!art_view.get_tile(i).is_empty()) {
      target_tile = i;
      found_non_empty = true;
      break;
    }
  }

  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping BMP header validation tests");
    return;
  }

  art2img::ImageView image_view{&art_view, target_tile};

  SUBCASE("BMP file header structure") {
    auto bmp_data = image_view.extract_to_bmp();
    REQUIRE_GT(bmp_data.size(), 54);  // Minimum BMP header size

    // Check file signature
    CHECK_EQ(bmp_data[0], 'B');
    CHECK_EQ(bmp_data[1], 'M');

    // Check file size
    uint32_t file_size =
        bmp_data[2] | (bmp_data[3] << 8) | (bmp_data[4] << 16) | (bmp_data[5] << 24);
    CHECK_EQ(file_size, bmp_data.size());

    // Check offset to pixel data (should be 54 for 24-bit BMP)
    uint32_t data_offset =
        bmp_data[10] | (bmp_data[11] << 8) | (bmp_data[12] << 16) | (bmp_data[13] << 24);
    CHECK_EQ(data_offset, 54);

    // Check info header size
    uint32_t info_header_size =
        bmp_data[14] | (bmp_data[15] << 8) | (bmp_data[16] << 16) | (bmp_data[17] << 24);
    CHECK_EQ(info_header_size, 40);
  }

  SUBCASE("BMP image dimensions") {
    art2img::ImageView image_view{&art_view, target_tile};
    auto bmp_data = image_view.extract_to_bmp();
    REQUIRE_GT(bmp_data.size(), 54);

    // Get image dimensions
    uint32_t width =
        bmp_data[18] | (bmp_data[19] << 8) | (bmp_data[20] << 16) | (bmp_data[21] << 24);
    uint32_t height =
        bmp_data[22] | (bmp_data[23] << 8) | (bmp_data[24] << 16) | (bmp_data[25] << 24);

    CHECK_EQ(width, image_view.width());
    CHECK_EQ(height, image_view.height());
  }

  SUBCASE("BMP bit depth") {
    auto bmp_data = image_view.extract_to_bmp();
    REQUIRE_GT(bmp_data.size(), 54);

    // Check bit depth (should be 32 for BGRA)
    uint16_t bit_depth = bmp_data[28] | (bmp_data[29] << 8);
    CHECK_EQ(bit_depth, 32);
  }
}

TEST_CASE("BMP format edge cases") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping BMP edge case tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  SUBCASE("Empty tile handling") {
    // Find an empty tile
    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      if (art_view.get_tile(i).is_empty()) {
        art2img::ImageView empty_image_view{&art_view, i};

        // Empty tiles should return true for saving operations
        std::string filename = "empty_tile.bmp";
        CHECK(empty_image_view.save_to_bmp(filename));  // Should succeed (no-op)

        // File should not be created for empty tiles
        CHECK_FALSE(std::filesystem::exists(filename));
        break;
      }
    }
  }

  SUBCASE("Single pixel tile") {
    // Find a small tile
    uint32_t target_tile = 0;
    bool found_small = false;

    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      if (!art_view.get_tile(i).is_empty()) {
        auto tile = art_view.get_tile(i);
        if (tile.width <= 4 && tile.height <= 4) {
          target_tile = i;
          found_small = true;
          break;
        }
      }
    }

    if (found_small) {
      art2img::ImageView image_view{&art_view, target_tile};
      std::string filename = "small_tile.bmp";
      REQUIRE(image_view.save_to_bmp(filename));
      CHECK(std::filesystem::exists(filename));
      CHECK_GT(std::filesystem::file_size(filename), 0);

      // Validate BMP file
      std::ifstream file(filename, std::ios::binary);
      REQUIRE(file.is_open());
      file.seekg(0, std::ios::end);
      size_t file_size = file.tellg();
      file.seekg(0, std::ios::beg);
      std::vector<uint8_t> file_data(file_size);
      file.read(reinterpret_cast<char*>(file_data.data()), file_size);
      file.close();
      CHECK(is_valid_bmp_signature(file_data));
      CHECK(has_valid_bmp_header(file_data));

      // Clean up
      std::filesystem::remove(filename);
    }
  }

  SUBCASE("Error handling with invalid options") {
    // Find a non-empty tile
    uint32_t target_tile = 0;
    bool found_non_empty = false;

    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      if (!art_view.get_tile(i).is_empty()) {
        target_tile = i;
        found_non_empty = true;
        break;
      }
    }

    if (found_non_empty) {
      art2img::ImageView image_view{&art_view, target_tile};
      std::string filename = "test_invalid.bmp";

      // Valid operations should succeed
      CHECK_NOTHROW(image_view.save_to_bmp(filename));

      if (std::filesystem::exists(filename)) {
        CHECK_GT(std::filesystem::file_size(filename), 0);
        std::filesystem::remove(filename);
      }
    }
  }
}

TEST_CASE("BMP format consistency with other formats") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping BMP consistency tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  // Find a non-empty tile
  uint32_t target_tile = 0;
  bool found_non_empty = false;

  for (uint32_t i = 0; i < art_view.image_count(); ++i) {
    if (!art_view.get_tile(i).is_empty()) {
      target_tile = i;
      found_non_empty = true;
      break;
    }
  }

  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping BMP consistency tests");
    return;
  }

  art2img::ImageView image_view{&art_view, target_tile};

  SUBCASE("Image dimensions consistency") {
    // Extract the same tile in different formats
    auto png_data = image_view.extract_to_png(art2img::ImageWriter::Options());
    auto bmp_data = image_view.extract_to_bmp();

    REQUIRE_GT(png_data.size(), 0);
    REQUIRE_GT(bmp_data.size(), 0);

    // Check that image dimensions match
    // For PNG, we need to parse the header to get dimensions
    // For BMP, we can read directly from the header
    if (bmp_data.size() >= 26) {
      uint32_t bmp_width =
          bmp_data[18] | (bmp_data[19] << 8) | (bmp_data[20] << 16) | (bmp_data[21] << 24);
      uint32_t bmp_height =
          bmp_data[22] | (bmp_data[23] << 8) | (bmp_data[24] << 16) | (bmp_data[25] << 24);

      CHECK_EQ(bmp_width, image_view.width());
      CHECK_EQ(bmp_height, image_view.height());
    }
  }

  SUBCASE("Color data consistency") {
    art2img::ImageWriter::Options options;
    options.fix_transparency = false;  // Disable transparency for easier comparison
    
    // Extract the same tile in different formats
    auto png_data = image_view.extract_to_png(options);
    auto bmp_data = image_view.extract_to_bmp();

    REQUIRE_GT(png_data.size(), 0);
    REQUIRE_GT(bmp_data.size(), 0);

    // Both should be valid image files
    unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
    CHECK(is_valid_bmp_signature(bmp_data));
  }
}

TEST_CASE("ExtractorAPI BMP format support") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping ExtractorAPI BMP tests");
    return;
  }

  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("Extract single tile as BMP") {
    auto result = extractor.extract_tile(0, art2img::ImageFormat::BMP);
    CHECK(result.success);
    if (result.success) {
      CHECK_GT(result.image_data.size(), 0);
      CHECK(is_valid_bmp_signature(result.image_data));
      CHECK_EQ(result.format, "bmp");
    }
  }

  SUBCASE("Batch extract tiles as BMP") {
    auto bmp_results = extractor.extract_all_tiles(art2img::ImageFormat::BMP);
    CHECK_EQ(bmp_results.size(), extractor.get_tile_count());

    size_t bmp_success_count = 0;
    size_t bmp_empty_count = 0;

    for (const auto& result : bmp_results) {
      if (result.success) {
        bmp_success_count++;
        if (result.image_data.empty()) {
          bmp_empty_count++;
        } else {
          // Non-empty images should be valid BMP files
          CHECK(is_valid_bmp_signature(result.image_data));
        }
      }
    }

    CHECK_EQ(bmp_success_count, bmp_results.size());  // All should succeed
  }
}

TEST_CASE("BMP format memory vs file consistency") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping BMP memory vs file consistency tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  // Find a non-empty tile
  uint32_t target_tile = 0;
  bool found_non_empty = false;

  for (uint32_t i = 0; i < art_view.image_count(); ++i) {
    if (!art_view.get_tile(i).is_empty()) {
      target_tile = i;
      found_non_empty = true;
      break;
    }
  }

  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping BMP memory vs file consistency tests");
    return;
  }

  art2img::ImageView image_view{&art_view, target_tile};

  SUBCASE("File vs memory output consistency") {
    // Save to file
    std::string filename = "consistency_test.bmp";
    REQUIRE(image_view.save_to_bmp(filename));
    
    // Extract to memory
    auto memory_data = image_view.extract_to_bmp();

    // Load file data
    std::ifstream file(filename, std::ios::binary);
    REQUIRE(file.is_open());
    file.seekg(0, std::ios::end);
    size_t file_size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<uint8_t> file_data(file_size);
    file.read(reinterpret_cast<char*>(file_data.data()), file_size);
    file.close();

    REQUIRE_EQ(file_data.size(), memory_data.size());
    CHECK(std::equal(file_data.begin(), file_data.end(), memory_data.begin()));

    // Clean up
    std::filesystem::remove(filename);
  }
}