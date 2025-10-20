#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "art2img/art_file.hpp"
#include "art2img/image_writer.hpp"
#include "art2img/palette.hpp"

// Include new module headers for direct testing
#include "art2img/file_operations.hpp"
#include "art2img/image_processor.hpp"

TEST_CASE("PNG Memory Regression - In-memory vs File output") {
  art2img::Palette palette;
  art2img::ArtFile::Tile tile{};
  tile.width = 1024;
  tile.height = 1024;
  tile.anim_data = 0;
  tile.offset = 0;

  const size_t pixel_count = static_cast<size_t>(tile.width) * tile.height;
  std::vector<uint8_t> pixel_data(pixel_count);
  for (size_t i = 0; i < pixel_count; ++i) {
    pixel_data[i] = static_cast<uint8_t>(i % 256);
  }

  // Test cases for module-specific PNG operations
  SUBCASE("Module-specific PNG operations") {
    art2img::Palette palette;
    art2img::ArtFile::Tile tile{};
    tile.width = 32;
    tile.height = 32;
    tile.anim_data = 0;
    tile.offset = 0;

    const size_t pixel_count = static_cast<size_t>(tile.width) * tile.height;
    std::vector<uint8_t> pixel_data(pixel_count);
    for (size_t i = 0; i < pixel_count; ++i) {
      pixel_data[i] = static_cast<uint8_t>(i % 256);
    }

    art2img::ImageWriter::Options options;

    SUBCASE("Image processor RGBA conversion") {
      // Test the image_processor module directly
      auto rgba_data = image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                        pixel_data.size(), options);
      CHECK_EQ(rgba_data.size(), pixel_count * 4);

      // Verify alpha values
      for (size_t i = 3; i < rgba_data.size(); i += 4) {
        CHECK((rgba_data[i] == 0 || rgba_data[i] == 255));
      }
    }

    SUBCASE("File operations PNG encoding") {
      // Test the file_operations module directly
      auto rgba_data = image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                        pixel_data.size(), options);
      auto png_data =
          art2img::file_operations::encode_png_to_memory(rgba_data, tile.width, tile.height);
      CHECK_GT(png_data.size(), 0);

      // Verify PNG signature
      REQUIRE_GE(png_data.size(), 8);
      unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
      CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
    }

    SUBCASE("Module pipeline vs ImageWriter") {
      // Compare ImageWriter vs direct module usage
      std::vector<uint8_t> image_writer_png;
      bool success = art2img::ImageWriter::write_image_to_memory(
          image_writer_png, art2img::ImageFormat::PNG, palette, tile, pixel_data.data(),
          pixel_data.size(), options);
      REQUIRE(success);
      CHECK_GT(image_writer_png.size(), 0);

      // Direct module usage
      auto rgba_data = image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                        pixel_data.size(), options);
      auto direct_png =
          art2img::file_operations::encode_png_to_memory(rgba_data, tile.width, tile.height);
      CHECK_GT(direct_png.size(), 0);

      // Both should produce valid PNGs
      REQUIRE_GE(image_writer_png.size(), 8);
      REQUIRE_GE(direct_png.size(), 8);
      unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
      CHECK(std::equal(png_header, png_header + 8, image_writer_png.begin()));
      CHECK(std::equal(png_header, png_header + 8, direct_png.begin()));
    }
  }

  art2img::ImageWriter::Options options;

  // Write to memory
  std::vector<uint8_t> memory_png;
  bool memory_success = art2img::ImageWriter::write_image_to_memory(
      memory_png, art2img::ImageFormat::PNG, palette, tile, pixel_data.data(), pixel_data.size(),
      options);

  CHECK(memory_success);
  CHECK(!memory_png.empty());

  if (memory_success && !memory_png.empty()) {
    // Write to file
    std::string disk_path = "build/tests/output/png_memory_regression_doctest.png";

    // Ensure output directory exists
    std::filesystem::create_directories("build/tests/output");

    bool file_success =
        art2img::ImageWriter::write_image(disk_path, art2img::ImageFormat::PNG, palette, tile,
                                          pixel_data.data(), pixel_data.size(), options);

    CHECK(file_success);

    if (file_success) {
      // Read the file back and compare
      std::ifstream file(disk_path, std::ios::binary);
      REQUIRE(file.is_open());

      std::vector<uint8_t> disk_png((std::istreambuf_iterator<char>(file)),
                                    std::istreambuf_iterator<char>());
      file.close();

      CHECK_EQ(memory_png.size(), disk_png.size());
      CHECK(memory_png == disk_png);
    }
  }
}