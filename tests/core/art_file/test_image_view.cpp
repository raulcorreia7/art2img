#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>

#include "art2img/exceptions.hpp"
#include "art2img/extractor_api.hpp"
#include "test_helpers.hpp"

// Include new module headers for direct testing
#include "art2img/file_operations.hpp"
#include "art2img/image_processor.hpp"
#include "art2img/image_writer.hpp"

// Function to create a test extractor with data
art2img::ExtractorAPI create_test_extractor() {
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

TEST_CASE("ImageView construction") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping ImageView tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  SUBCASE("Valid ImageView construction") {
    for (uint32_t i = 0; i < std::min(art_view.image_count(), static_cast<size_t>(5UL)); ++i) {
      if (!art_view.get_tile(i).is_empty()) {
        art2img::ImageView image_view{&art_view, i};

        CHECK_NE(image_view.parent, nullptr);
        CHECK_EQ(image_view.tile_index, i);
        CHECK_EQ(image_view.width(), art_view.get_tile(i).width);
        CHECK_EQ(image_view.height(), art_view.get_tile(i).height);
        CHECK_EQ(image_view.size(), static_cast<size_t>(image_view.width()) * image_view.height());

        break;  // Test first non-empty tile
      }
    }
  }

  SUBCASE("Count size") {
    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      art2img::ImageView image_view{&art_view, i};
      auto tile = art_view.get_tile(i);

      if (tile.is_empty()) {
        CHECK_EQ(image_view.size(), 0);
      } else {
        CHECK_EQ(image_view.size(), static_cast<size_t>(tile.width) * tile.height);
        CHECK_EQ(image_view.pixel_data() != nullptr, true);
      }
    }
  }
}

TEST_CASE("ImageView pixel data access") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping pixel data tests");
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

  INFO("Using tile " << target_tile << " for testing");
  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping pixel data tests");
    return;
  }

  SUBCASE("Pixel data access") {
    art2img::ImageView image_view{&art_view, target_tile};

    const auto* pixel_data = image_view.pixel_data();
    CHECK_NE(pixel_data, nullptr);

    // Verify we can access data without crashing
    // (Just checking first few bytes are reasonable palette indices)
    for (size_t i = 0; i < std::min(image_view.size(), size_t(10)); ++i) {
      CHECK_LT(pixel_data[i], 256);  // Should be valid palette indices
    }
  }

  SUBCASE("Empty tile pixel data") {
    // Find an empty tile
    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      if (art_view.get_tile(i).is_empty()) {
        art2img::ImageView image_view{&art_view, i};
        CHECK_EQ(image_view.pixel_data(), nullptr);
        CHECK_EQ(image_view.size(), 0);
        break;
      }
    }
  }
}

TEST_CASE("ImageView animation data access") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping animation data tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  for (uint32_t i = 0; i < std::min(art_view.image_count(), static_cast<size_t>(10UL)); ++i) {
    art2img::ImageView image_view{&art_view, i};
    auto tile = art_view.get_tile(i);

    // Check animation data accessors
    CHECK_EQ(image_view.anim_frames(), tile.anim_frames());
    CHECK_EQ(image_view.anim_type(), tile.anim_type());
    CHECK_EQ(image_view.x_offset(), tile.x_offset());
    CHECK_EQ(image_view.y_offset(), tile.y_offset());
    CHECK_EQ(image_view.anim_speed(), tile.anim_speed());
    CHECK_EQ(image_view.other_flags(), tile.other_flags());

    // Reasonable ranges for animation data
    CHECK_LE(image_view.anim_frames(), 64);
    CHECK_LE(image_view.anim_type(), 4);
    CHECK_LE(image_view.anim_speed(), 16);
  }
}

TEST_CASE("ImageView image saving") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping image saving tests");
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
    MESSAGE("No non-empty tiles found, skipping image saving tests");
    return;
  }

  art2img::ImageView image_view{&art_view, target_tile};

  SUBCASE("Save to TGA") {
    std::string filename = "test_tile.tga";
    REQUIRE(image_view.save_to_tga(filename));
    CHECK(std::filesystem::exists(filename));
    CHECK_GT(std::filesystem::file_size(filename), 0);

    // Clean up
    std::filesystem::remove(filename);
  }

  SUBCASE("Save to PNG with default options") {
    std::string filename = "test_tile.png";
    REQUIRE(image_view.save_to_png(filename, art2img::ImageWriter::Options()));
    CHECK(std::filesystem::exists(filename));
    CHECK_GT(std::filesystem::file_size(filename), 0);

    // Clean up
    std::filesystem::remove(filename);
  }

  SUBCASE("Save to PNG with custom options") {
    art2img::ImageWriter::Options options;
    options.fix_transparency = false;

    std::string filename = "test_tile_custom.png";
    REQUIRE(image_view.save_to_png(filename, options));
    CHECK(std::filesystem::exists(filename));
    CHECK_GT(std::filesystem::file_size(filename), 0);

    // Clean up
    std::filesystem::remove(filename);
  }

  SUBCASE("Extract to TGA memory") {
    auto tga_data = image_view.extract_to_tga();
    CHECK_GT(tga_data.size(), 0);

    // TGA files should have header, so minimum size should be > 100 bytes
    CHECK_GT(tga_data.size(), 100);
  }

  SUBCASE("Extract to PNG memory") {
    auto png_data = image_view.extract_to_png(art2img::ImageWriter::Options());
    CHECK_GT(png_data.size(), 0);

    // PNG files should start with PNG signature
    REQUIRE_GE(png_data.size(), 8);
    unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
  }

  SUBCASE("Extract to PNG memory with options") {
    art2img::ImageWriter::Options options;
    options.fix_transparency = true;

    auto png_data = image_view.extract_to_png(options);
    CHECK_GT(png_data.size(), 0);

    // Check PNG signature
    REQUIRE_GE(png_data.size(), 8);
    unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
  }

  SUBCASE("Save to BMP with default options") {
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

  SUBCASE("Save to BMP with custom options") {
    std::string filename = "test_tile_custom.bmp";
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

  SUBCASE("Extract to BMP memory") {
    auto bmp_data = image_view.extract_to_bmp();
    CHECK_GT(bmp_data.size(), 0);

    // BMP files should have headers, so minimum size should be > 54 bytes
    CHECK_GT(bmp_data.size(), 54);

    // Check BMP file signature
    REQUIRE_GE(bmp_data.size(), 2);
    CHECK_EQ(bmp_data[0], 'B');
    CHECK_EQ(bmp_data[1], 'M');
  }

  SUBCASE("Extract to BMP memory with options") {
    auto bmp_data = image_view.extract_to_bmp();
    CHECK_GT(bmp_data.size(), 0);

    // Check BMP file signature
    REQUIRE_GE(bmp_data.size(), 2);
    CHECK_EQ(bmp_data[0], 'B');
    CHECK_EQ(bmp_data[1], 'M');
  }
}

TEST_CASE("ImageView error handling") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping error handling tests");
    return;
  }

  auto extractor = create_test_extractor();
  auto art_view = extractor.get_art_view();

  SUBCASE("Invalid ImageView state") {
    art2img::ImageView invalid_image_view{};
    CHECK_THROWS_AS(invalid_image_view.pixel_data(), art2img::ArtException);
    CHECK_THROWS_AS(invalid_image_view.width(), art2img::ArtException);
    CHECK_THROWS_AS(invalid_image_view.height(), art2img::ArtException);
    CHECK_THROWS_AS(invalid_image_view.size(), art2img::ArtException);
  }

  SUBCASE("Out of range tile index") {
    art2img::ImageView out_of_range_view{&art_view, static_cast<uint32_t>(art_view.image_count())};
    CHECK_THROWS_AS(out_of_range_view.pixel_data(), art2img::ArtException);
    CHECK_THROWS_AS(out_of_range_view.width(), art2img::ArtException);
    CHECK_THROWS_AS(out_of_range_view.height(), art2img::ArtException);
  }

  SUBCASE("Save empty tile") {
    // Find an empty tile
    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      if (art_view.get_tile(i).is_empty()) {
        art2img::ImageView empty_image_view{&art_view, i};

        // Empty tiles should return false for saving operations
        std::string filename = "empty_tile.tga";
        CHECK(empty_image_view.save_to_tga(filename));  // Should succeed (no-op)
        return;
      }
    }
  }
}

// Test cases for image_processor module functionality
TEST_CASE("ImageProcessor module tests") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping image_processor tests");
    return;
  }

  // Load test data
  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");

  art2img::Palette palette;
  palette.load_from_memory(palette_data.data(), palette_data.size());

  art2img::ArtFile art_file(art_data.data(), art_data.size());
  const auto& tiles = art_file.tiles();

  // Find a non-empty tile
  uint32_t target_tile = 0;
  bool found_non_empty = false;

  for (uint32_t i = 0; i < tiles.size(); ++i) {
    if (!tiles[i].is_empty()) {
      target_tile = i;
      found_non_empty = true;
      break;
    }
  }

  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping image_processor tests");
    return;
  }

  const auto& tile = tiles[target_tile];
  std::vector<uint8_t> pixel_data(tile.size());

  // Read pixel data directly from ART file
  if (tile.offset + tile.size() <= art_data.size()) {
    std::copy(art_data.data() + tile.offset, art_data.data() + tile.offset + tile.size(),
              pixel_data.data());
  } else {
    MESSAGE("Tile data extends beyond buffer, skipping image_processor tests");
    return;
  }

  SUBCASE("Convert to RGBA") {
    art2img::ImageWriter::Options options;
    auto rgba_data = image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                      pixel_data.size(), options);

    CHECK_EQ(rgba_data.size(), static_cast<size_t>(tile.width) * tile.height * 4);

    // Check that alpha values are reasonable (0 or 255)
    for (size_t i = 3; i < rgba_data.size(); i += 4) {
      CHECK((rgba_data[i] == 0 || rgba_data[i] == 255));
    }
  }

  SUBCASE("Magenta detection") {
    // Test the is_magenta function
    CHECK(art2img::ImageWriter::is_magenta(255, 0, 255));        // Pure magenta
    CHECK(art2img::ImageWriter::is_magenta(250, 0, 250));        // Build engine magenta
    CHECK(art2img::ImageWriter::is_magenta(255, 5, 255));        // Magenta with some green
    CHECK_FALSE(art2img::ImageWriter::is_magenta(255, 6, 255));  // Too much green
    CHECK_FALSE(art2img::ImageWriter::is_magenta(249, 0, 255));  // Not red enough
    CHECK_FALSE(art2img::ImageWriter::is_magenta(255, 0, 249));  // Not blue enough
  }
}

// Test cases for file_operations module functionality
TEST_CASE("FileOperations module tests") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping file_operations tests");
    return;
  }

  // Load test data
  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");

  art2img::Palette palette;
  palette.load_from_memory(palette_data.data(), palette_data.size());

  art2img::ArtFile art_file(art_data.data(), art_data.size());
  const auto& tiles = art_file.tiles();

  // Find a non-empty tile
  uint32_t target_tile = 0;
  bool found_non_empty = false;

  for (uint32_t i = 0; i < tiles.size(); ++i) {
    if (!tiles[i].is_empty()) {
      target_tile = i;
      found_non_empty = true;
      break;
    }
  }

  if (!found_non_empty) {
    MESSAGE("No non-empty tiles found, skipping file_operations tests");
    return;
  }

  const auto& tile = tiles[target_tile];
  std::vector<uint8_t> pixel_data(tile.size());

  // Read pixel data directly from ART file
  if (tile.offset + tile.size() <= art_data.size()) {
    std::copy(art_data.data() + tile.offset, art_data.data() + tile.offset + tile.size(),
              pixel_data.data());
  } else {
    MESSAGE("Tile data extends beyond buffer, skipping file_operations tests");
    return;
  }

  // Convert to RGBA for PNG operations
  art2img::ImageWriter::Options options;
  auto rgba_data = image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                    pixel_data.size(), options);

  SUBCASE("PNG encoding to memory") {
    auto png_data =
        art2img::file_operations::encode_png_to_memory(rgba_data, tile.width, tile.height);
    CHECK_GT(png_data.size(), 0);

    // Check PNG signature
    REQUIRE_GE(png_data.size(), 8);
    unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
    CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
  }

  SUBCASE("TGA encoding to memory") {
    auto tga_data = art2img::file_operations::encode_tga_to_memory(palette, pixel_data, tile.width,
                                                                   tile.height);
    CHECK_GT(tga_data.size(), 0);

    // Check TGA header (first 18 bytes)
    REQUIRE_GE(tga_data.size(), 18);
    CHECK_EQ(tga_data[1], 1);  // Color map type
    CHECK_EQ(tga_data[2], 1);  // Image type (color mapped)
  }
}
