/// @file test_encode.cpp
/// @brief Unit tests for image encoding functions

#include <cstring>
#include <vector>

#include <doctest/doctest.h>

#include <art2img/convert.hpp>
#include <art2img/encode.hpp>

using namespace art2img;

namespace {

/// @brief Create a simple test image with known pixel values
Image create_test_image(u16 width, u16 height) {
  Image image(width, height);

  // Fill with a simple pattern: red gradient from left to right
  for (u16 y = 0; y < height; ++y) {
    for (u16 x = 0; x < width; ++x) {
      const std::size_t offset = (static_cast<std::size_t>(y) * width + x) * 4;
      image.data[offset + 0] = static_cast<u8>(x * 255 / width);   // Red
      image.data[offset + 1] = static_cast<u8>(y * 255 / height);  // Green
      image.data[offset + 2] = 128;                                // Blue
      image.data[offset + 3] = 255;                                // Alpha
    }
  }

  return image;
}

/// @brief Check if encoded data has valid format headers
bool check_png_header(const std::vector<byte>& data) {
  return data.size() >= 8 && data[0] == static_cast<byte>(0x89) &&
         data[1] == static_cast<byte>(0x50) &&  // 'P'
         data[2] == static_cast<byte>(0x4E) &&  // 'N'
         data[3] == static_cast<byte>(0x47) &&  // 'G'
         data[4] == static_cast<byte>(0x0D) &&
         data[5] == static_cast<byte>(0x0A) &&
         data[6] == static_cast<byte>(0x1A) &&
         data[7] == static_cast<byte>(0x0A);
}

bool check_tga_header(const std::vector<byte>& data) {
  return data.size() >= 18 &&
         (data[2] == static_cast<byte>(2) ||
          data[2] == static_cast<byte>(10));  // Uncompressed or RLE compressed
}

bool check_bmp_header(const std::vector<byte>& data) {
  return data.size() >= 54 && data[0] == static_cast<byte>(0x42) &&  // 'B'
         data[1] == static_cast<byte>(0x4D);                         // 'M'
}

}  // anonymous namespace

TEST_SUITE("encode") {
  // ============================================================================
  // VALIDATION TESTS
  // ============================================================================

  TEST_CASE("validate_image_for_encoding - valid image") {
    const Image image = create_test_image(64, 64);
    const ImageView view = image_view(image);

    const auto result = validate_image_for_encoding(view);
    REQUIRE(result.has_value());
  }

  TEST_CASE("validate_image_for_encoding - invalid stride") {
    Image image = create_test_image(64, 64);
    // Corrupt the stride to make it invalid
    image.stride = 100;
    const ImageView view(image.data, image.width, image.height, image.stride);

    const auto result = validate_image_for_encoding(view);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::encoding_failure);
  }

  TEST_CASE("validate_image_for_encoding - invalid dimensions") {
    Image image = create_test_image(0, 64);  // Zero width
    const ImageView view = image_view(image);

    const auto result = validate_image_for_encoding(view);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::encoding_failure);
  }

  // ============================================================================
  // PNG ENCODING TESTS
  // ============================================================================

  TEST_CASE("encode_png - basic encoding") {
    const Image image = create_test_image(32, 32);
    const ImageView view = image_view(image);

    const auto result = encode_png(view);
    REQUIRE(result.has_value());

    const std::vector<byte>& encoded = result.value();
    CHECK(encoded.size() > 100);  // PNG should have some reasonable size
    CHECK(check_png_header(encoded));
  }

  TEST_CASE("encode_png - with options") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    PngOptions options;
    options.compression_level = 9;
    options.use_filters = false;
    options.convert_to_grayscale = false;

    const auto result = encode_png(view, options);
    REQUIRE(result.has_value());
    CHECK(check_png_header(result.value()));
  }

  TEST_CASE("encode_png - grayscale conversion") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    PngOptions options;
    options.convert_to_grayscale = true;

    const auto result = encode_png(view, options);
    REQUIRE(result.has_value());
    CHECK(check_png_header(result.value()));
  }

  // ============================================================================
  // TGA ENCODING TESTS
  // ============================================================================

  TEST_CASE("encode_tga - basic encoding") {
    const Image image = create_test_image(32, 32);
    const ImageView view = image_view(image);

    const auto result = encode_tga(view);
    REQUIRE(result.has_value());

    const std::vector<byte>& encoded = result.value();
    CHECK(encoded.size() > 50);  // TGA should have some reasonable size
    CHECK(check_tga_header(encoded));
  }

  TEST_CASE("encode_tga - with options") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    TgaOptions options;
    options.use_rle = false;
    options.include_alpha = false;
    options.flip_vertically = true;

    const auto result = encode_tga(view, options);
    REQUIRE(result.has_value());
    CHECK(check_tga_header(result.value()));
  }

  // ============================================================================
  // BMP ENCODING TESTS
  // ============================================================================

  TEST_CASE("encode_bmp - basic encoding") {
    const Image image = create_test_image(32, 32);
    const ImageView view = image_view(image);

    const auto result = encode_bmp(view);
    REQUIRE(result.has_value());

    const std::vector<byte>& encoded = result.value();
    CHECK(encoded.size() > 100);  // BMP should have some reasonable size
    CHECK(check_bmp_header(encoded));
  }

  TEST_CASE("encode_bmp - with options") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    BmpOptions options;
    options.include_alpha = false;
    options.flip_vertically = false;

    const auto result = encode_bmp(view, options);
    REQUIRE(result.has_value());
    CHECK(check_bmp_header(result.value()));
  }

  // ============================================================================
  // GENERIC ENCODING TESTS
  // ============================================================================

  TEST_CASE("encode_image - PNG format") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    const auto result = encode_image(view, ImageFormat::png);
    REQUIRE(result.has_value());
    CHECK(check_png_header(result.value()));
  }

  TEST_CASE("encode_image - TGA format") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    const auto result = encode_image(view, ImageFormat::tga);
    REQUIRE(result.has_value());
    CHECK(check_tga_header(result.value()));
  }

  TEST_CASE("encode_image - BMP format") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    const auto result = encode_image(view, ImageFormat::bmp);
    REQUIRE(result.has_value());
    CHECK(check_bmp_header(result.value()));
  }

  TEST_CASE("encode_image - invalid options for format") {
    const Image image = create_test_image(16, 16);
    const ImageView view = image_view(image);

    // Pass TGA options for PNG encoding
    const auto result = encode_image(view, ImageFormat::png, TgaOptions{});
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::encoding_failure);
  }

  // ============================================================================
  // UTILITY FUNCTION TESTS
  // ============================================================================

  TEST_CASE("get_default_options") {
    const auto png_opts = get_default_options(ImageFormat::png);
    CHECK(std::holds_alternative<PngOptions>(png_opts));

    const auto tga_opts = get_default_options(ImageFormat::tga);
    CHECK(std::holds_alternative<TgaOptions>(tga_opts));

    const auto bmp_opts = get_default_options(ImageFormat::bmp);
    CHECK(std::holds_alternative<BmpOptions>(bmp_opts));
  }

  TEST_CASE("format_to_string") {
    CHECK(std::string(format_to_string(ImageFormat::png)) == "PNG");
    CHECK(std::string(format_to_string(ImageFormat::tga)) == "TGA");
    CHECK(std::string(format_to_string(ImageFormat::bmp)) == "BMP");
  }

  // ============================================================================
  // ERROR HANDLING TESTS
  // ============================================================================

  TEST_CASE("encode functions - invalid image") {
    // Create an image with invalid stride
    Image image = create_test_image(16, 16);
    image.stride = 1;  // Invalid stride
    const ImageView view(image.data, image.width, image.height, image.stride);

    const auto png_result = encode_png(view);
    REQUIRE(!png_result.has_value());
    CHECK(png_result.error().code == errc::encoding_failure);

    const auto tga_result = encode_tga(view);
    REQUIRE(!tga_result.has_value());
    CHECK(tga_result.error().code == errc::encoding_failure);

    const auto bmp_result = encode_bmp(view);
    REQUIRE(!bmp_result.has_value());
    CHECK(bmp_result.error().code == errc::encoding_failure);
  }

  // ============================================================================
  // SIZE AND DIMENSION TESTS
  // ============================================================================

  TEST_CASE("encode_different_sizes") {
    const std::vector<std::pair<u16, u16>> sizes = {
        {1, 1}, {8, 8}, {16, 16}, {32, 32}, {64, 64}, {128, 128}};

    for (const auto& [width, height] : sizes) {
      const Image image = create_test_image(width, height);
      const ImageView view = image_view(image);

      const auto png_result = encode_png(view);
      REQUIRE(png_result.has_value());
      CHECK(check_png_header(png_result.value()));

      const auto tga_result = encode_tga(view);
      REQUIRE(tga_result.has_value());
      CHECK(check_tga_header(tga_result.value()));

      const auto bmp_result = encode_bmp(view);
      REQUIRE(bmp_result.has_value());
      CHECK(check_bmp_header(bmp_result.value()));
    }
  }

}  // TEST_SUITE("encode")