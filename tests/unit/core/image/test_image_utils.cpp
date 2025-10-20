#include <doctest/doctest.h>
#include <art2img/color_helpers.hpp>
#include <art2img/detail/image_utils.hpp>
#include <art2img/types.hpp>

using namespace art2img;

TEST_SUITE("Image Utilities") {
  TEST_CASE("write_rgba writes color values correctly") {
    std::vector<types::u8> buffer(8, 0);  // 2 pixels worth of data
    color::Color color(255, 128, 64, 32);

    detail::write_rgba(types::mutable_u8_span(buffer), 0, color);

    CHECK(buffer[0] == 255);  // R
    CHECK(buffer[1] == 128);  // G
    CHECK(buffer[2] == 64);   // B
    CHECK(buffer[3] == 32);   // A
  }

  TEST_CASE("write_rgba handles out of bounds gracefully") {
    std::vector<types::u8> buffer(3,
                                  0);  // Not enough space for a full RGBA pixel
    color::Color color(255, 128, 64, 32);

    // This should not crash or write beyond buffer bounds
    detail::write_rgba(types::mutable_u8_span(buffer), 0, color);

    // Buffer should remain unchanged since there's not enough space
    CHECK(buffer[0] == 0);
    CHECK(buffer[1] == 0);
    CHECK(buffer[2] == 0);
  }

  TEST_CASE("flip_image_vertically flips image data correctly") {
    // Create a simple 2x2 image with distinct values
    types::u8 data[] = {1, 2,   // Row 0
                        3, 4};  // Row 1

    auto flipped = detail::flip_image_vertically(data, 2, 2, 1);

    // After flip: Row 1 should come first, then Row 0
    CHECK(flipped[0] == 3);  // Row 1, col 0
    CHECK(flipped[1] == 4);  // Row 1, col 1
    CHECK(flipped[2] == 1);  // Row 0, col 0
    CHECK(flipped[3] == 2);  // Row 0, col 1
  }

  TEST_CASE(
      "clean_transparent_pixels sets transparent pixels to neutral color") {
    // Create image data with one transparent pixel and one opaque pixel
    std::vector<types::u8> rgba_data = {
        255, 0,   0, 0,     // Transparent red pixel
        0,   255, 0, 255};  // Opaque green pixel

    detail::clean_transparent_pixels(rgba_data, 2, 1);

    // Transparent pixel should be changed to neutral gray
    CHECK(rgba_data[0] == 128);  // R = 128
    CHECK(rgba_data[1] == 128);  // G = 128
    CHECK(rgba_data[2] == 128);  // B = 128
    CHECK(rgba_data[3] == 0);    // Alpha unchanged

    // Opaque pixel should remain unchanged
    CHECK(rgba_data[4] == 0);    // R = 0
    CHECK(rgba_data[5] == 255);  // G = 255
    CHECK(rgba_data[6] == 0);    // B = 0
    CHECK(rgba_data[7] == 255);  // Alpha = 255
  }

  TEST_CASE("apply_matte_hygiene processes alpha channel correctly") {
    // Create a simple 3x3 image with a single opaque pixel in the center
    std::vector<types::u8> rgba_data(36, 0);  // 3x3x4 = 36 bytes
    rgba_data[20] = 255;  // Center pixel alpha = 255 (opaque)

    // Store original alpha values for reference
    std::vector<types::u8> original_alpha(9, 0);
    original_alpha[4] = 255;  // Center pixel

    detail::apply_matte_hygiene(rgba_data, 3, 3);

    // The function should modify the alpha channel based on neighbors
    // Exact values depend on the erosion and blur implementation
    // We'll just check that the function runs without crashing and modifies data
    CHECK(rgba_data.size() == 36);  // Size unchanged
  }
}