#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <vector>

#include "image_processor.hpp"
#include "image_writer.hpp"
#include "palette.hpp"

// Test transparency functionality with both file and memory operations
TEST_CASE("Transparency processing - magenta pixel detection and alpha channel handling") {
  // Create a simple palette with magenta colors
  art2img::Palette palette;
  palette.load_duke3d_default();

  // Modify palette to have specific test colors
  auto raw_data = palette.raw_data();

  // Set palette index 254 to pure magenta (255, 0, 255) in BGR format
  raw_data[254 * 3 + 0] = 255;  // B
  raw_data[254 * 3 + 1] = 0;    // G
  raw_data[254 * 3 + 2] = 255;  // R

  // Set palette index 253 to non-magenta color (200, 100, 150) in BGR format
  raw_data[253 * 3 + 0] = 150;  // B
  raw_data[253 * 3 + 1] = 100;  // G
  raw_data[253 * 3 + 2] = 200;  // R

  palette.load_from_memory(raw_data.data(), raw_data.size());

  // Create a simple 2x2 tile
  art2img::ArtFile::Tile tile{};
  tile.width = 2;
  tile.height = 2;
  tile.anim_data = 0;
  tile.offset = 0;

  // Create pixel data with magenta and non-magenta pixels
  std::vector<uint8_t> pixel_data = {
      254,  // Magenta pixel
      253,  // Non-magenta pixel
      253,  // Non-magenta pixel
      254   // Magenta pixel
  };

  SUBCASE("Magenta pixels become transparent when fix_transparency is enabled") {
    art2img::ImageWriter::Options options;
    options.fix_transparency = true;
    options.enable_alpha = true;

    // Convert to RGBA directly
    auto rgba_data = art2img::image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                               pixel_data.size(), options);

    REQUIRE_EQ(rgba_data.size(), 16);  // 2x2x4 = 16 bytes

    // Check that magenta pixels (index 254) have alpha=0
    CHECK_EQ(rgba_data[3], 0);     // First pixel alpha = 0 (transparent)
    CHECK_EQ(rgba_data[7], 255);   // Second pixel alpha = 255 (opaque)
    CHECK_EQ(rgba_data[11], 255);  // Third pixel alpha = 255 (opaque)
    CHECK_EQ(rgba_data[15], 0);    // Fourth pixel alpha = 0 (transparent)
  }

  SUBCASE("All pixels remain opaque when fix_transparency is disabled") {
    art2img::ImageWriter::Options options;
    options.fix_transparency = false;
    options.enable_alpha = true;

    // Convert to RGBA directly
    auto rgba_data = art2img::image_processor::convert_to_rgba(palette, tile, pixel_data.data(),
                                                               pixel_data.size(), options);

    REQUIRE_EQ(rgba_data.size(), 16);  // 2x2x4 = 16 bytes

    // All pixels should be opaque (no transparency processing)
    CHECK_EQ(rgba_data[3], 255);   // First pixel alpha = 255 (opaque)
    CHECK_EQ(rgba_data[7], 255);   // Second pixel alpha = 255 (opaque)
    CHECK_EQ(rgba_data[11], 255);  // Third pixel alpha = 255 (opaque)
    CHECK_EQ(rgba_data[15], 255);  // Fourth pixel alpha = 255 (opaque)
  }
}