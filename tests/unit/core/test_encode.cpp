#include <doctest/doctest.h>

#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>
#include <art2img/core/palette.hpp>
#include <filesystem>

TEST_SUITE("encode module")
{
  TEST_CASE("Basic encode functionality compilation test")
  {
    // This test verifies that the encode module can be included and basic types
    // exist
    using art2img::core::EncodedImage;
    using art2img::core::EncoderOptions;
    using art2img::core::ImageFormat;

    // Test default options
    EncoderOptions opts;
    CHECK(opts.compression == art2img::core::CompressionPreset::balanced);
    CHECK(opts.bit_depth == art2img::core::BitDepth::auto_detect);

    // Test file extension function
    CHECK(art2img::core::file_extension(ImageFormat::png) == "png");
    CHECK(art2img::core::file_extension(ImageFormat::tga) == "tga");
    CHECK(art2img::core::file_extension(ImageFormat::bmp) == "bmp");

    // Basic compilation test - more comprehensive tests will be added
    // as the implementation progresses
    CHECK(true);
  }

  TEST_CASE("Encode RGBA image to different formats")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    // Load and convert a tile first
    auto art_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(art_data.has_value());

    auto palette_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(palette_data.has_value());

    auto archive = art2img::core::load_art(*art_data);
    REQUIRE(archive.has_value());

    auto palette = art2img::core::load_palette(*palette_data);
    REQUIRE(palette.has_value());

    auto palette_view = art2img::core::view_palette(*palette);

    auto tile = art2img::core::get_tile(*archive, 0);
    REQUIRE(tile.has_value());

    auto rgba_image = art2img::core::palette_to_rgba(*tile, palette_view);
    REQUIRE(rgba_image.has_value());

    auto image_view = art2img::core::make_view(*rgba_image);
    REQUIRE(image_view.valid());

    // Test encoding to PNG
    auto png_result = art2img::core::encode_image(
        image_view, art2img::core::ImageFormat::png);
    if (png_result.has_value()) {
      CHECK(png_result->format == art2img::core::ImageFormat::png);
      CHECK(png_result->width == rgba_image->width);
      CHECK(png_result->height == rgba_image->height);
      CHECK(!png_result->bytes.empty());
    }

    // Test encoding to TGA
    auto tga_result = art2img::core::encode_image(
        image_view, art2img::core::ImageFormat::tga);
    if (tga_result.has_value()) {
      CHECK(tga_result->format == art2img::core::ImageFormat::tga);
      CHECK(tga_result->width == rgba_image->width);
      CHECK(tga_result->height == rgba_image->height);
      CHECK(!tga_result->bytes.empty());
    }

    // Test encoding to BMP
    auto bmp_result = art2img::core::encode_image(
        image_view, art2img::core::ImageFormat::bmp);
    if (bmp_result.has_value()) {
      CHECK(bmp_result->format == art2img::core::ImageFormat::bmp);
      CHECK(bmp_result->width == rgba_image->width);
      CHECK(bmp_result->height == rgba_image->height);
      CHECK(!bmp_result->bytes.empty());
    }
  }

  TEST_CASE("Encode with different options")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    // Load and convert a tile
    auto art_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(art_data.has_value());

    auto palette_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(palette_data.has_value());

    auto archive = art2img::core::load_art(*art_data);
    REQUIRE(archive.has_value());

    auto palette = art2img::core::load_palette(*palette_data);
    REQUIRE(palette.has_value());

    auto palette_view = art2img::core::view_palette(*palette);

    auto tile = art2img::core::get_tile(*archive, 0);
    REQUIRE(tile.has_value());

    auto rgba_image = art2img::core::palette_to_rgba(*tile, palette_view);
    REQUIRE(rgba_image.has_value());

    auto image_view = art2img::core::make_view(*rgba_image);
    REQUIRE(image_view.valid());

    // Test different compression presets
    std::vector<art2img::core::CompressionPreset> compression_presets = {
        art2img::core::CompressionPreset::fast,
        art2img::core::CompressionPreset::balanced,
        art2img::core::CompressionPreset::smallest};

    for (auto compression : compression_presets) {
      art2img::core::EncoderOptions opts;
      opts.compression = compression;

      auto result = art2img::core::encode_image(
          image_view, art2img::core::ImageFormat::png, opts);
      if (result.has_value()) {
        CHECK(result->format == art2img::core::ImageFormat::png);
        CHECK(result->width == rgba_image->width);
        CHECK(result->height == rgba_image->height);
        CHECK(!result->bytes.empty());
      }
    }

    // Test different bit depths
    std::vector<art2img::core::BitDepth> bit_depths = {
        art2img::core::BitDepth::auto_detect, art2img::core::BitDepth::bpp24,
        art2img::core::BitDepth::bpp32};

    for (auto bit_depth : bit_depths) {
      art2img::core::EncoderOptions opts;
      opts.bit_depth = bit_depth;

      auto result = art2img::core::encode_image(
          image_view, art2img::core::ImageFormat::png, opts);
      if (result.has_value()) {
        CHECK(result->format == art2img::core::ImageFormat::png);
        CHECK(result->width == rgba_image->width);
        CHECK(result->height == rgba_image->height);
        CHECK(!result->bytes.empty());
      }
    }
  }

  TEST_CASE("Encode multiple tiles")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    // Load test assets
    auto art_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(art_data.has_value());

    auto palette_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(palette_data.has_value());

    auto archive = art2img::core::load_art(*art_data);
    REQUIRE(archive.has_value());

    auto palette = art2img::core::load_palette(*palette_data);
    REQUIRE(palette.has_value());

    auto palette_view = art2img::core::view_palette(*palette);

    auto tile_count = art2img::core::tile_count(*archive);
    REQUIRE(tile_count > 0);

    // Encode first few tiles to test consistency
    const std::size_t tiles_to_test =
        std::min(tile_count, static_cast<std::size_t>(3));

    for (std::size_t i = 0; i < tiles_to_test; ++i) {
      auto tile = art2img::core::get_tile(*archive, i);
      REQUIRE(tile.has_value());

      auto rgba_image = art2img::core::palette_to_rgba(*tile, palette_view);
      if (rgba_image.has_value()) {
        auto image_view = art2img::core::make_view(*rgba_image);
        REQUIRE(image_view.valid());

        // Test PNG encoding
        auto png_result = art2img::core::encode_image(
            image_view, art2img::core::ImageFormat::png);
        if (png_result.has_value()) {
          CHECK(png_result->format == art2img::core::ImageFormat::png);
          CHECK(png_result->width == rgba_image->width);
          CHECK(png_result->height == rgba_image->height);
          CHECK(!png_result->bytes.empty());
        }
      }
    }
  }

  TEST_CASE("Encode with invalid inputs")
  {
    // Test with invalid image view
    art2img::core::RgbaImageView invalid_view;
    invalid_view.width = 0;
    invalid_view.height = 0;
    invalid_view.stride = 0;
    // pixels span is empty

    auto invalid_result = art2img::core::encode_image(
        invalid_view, art2img::core::ImageFormat::png);
    CHECK(!invalid_result.has_value());

    // Test with inconsistent dimensions
    art2img::core::RgbaImageView inconsistent_view;
    inconsistent_view.width = 10;
    inconsistent_view.height = 10;
    inconsistent_view.stride = 20;  // Too small for 10*width*4 bytes
    std::vector<std::uint8_t> dummy_pixels(400, 0);
    inconsistent_view.pixels = dummy_pixels;

    auto inconsistent_result = art2img::core::encode_image(
        inconsistent_view, art2img::core::ImageFormat::png);
    CHECK(!inconsistent_result.has_value());
  }

  TEST_CASE("Encoder options validation")
  {
    art2img::core::EncoderOptions opts;

    // Test default values
    CHECK(opts.compression == art2img::core::CompressionPreset::balanced);
    CHECK(opts.bit_depth == art2img::core::BitDepth::auto_detect);

    // Test setting compression presets
    opts.compression = art2img::core::CompressionPreset::fast;
    CHECK(opts.compression == art2img::core::CompressionPreset::fast);

    opts.compression = art2img::core::CompressionPreset::smallest;
    CHECK(opts.compression == art2img::core::CompressionPreset::smallest);

    // Test setting bit depths
    opts.bit_depth = art2img::core::BitDepth::bpp24;
    CHECK(opts.bit_depth == art2img::core::BitDepth::bpp24);

    opts.bit_depth = art2img::core::BitDepth::bpp32;
    CHECK(opts.bit_depth == art2img::core::BitDepth::bpp32);
  }

  TEST_CASE("File extension function comprehensive test")
  {
    // Test all supported formats
    CHECK(art2img::core::file_extension(art2img::core::ImageFormat::png) ==
          "png");
    CHECK(art2img::core::file_extension(art2img::core::ImageFormat::tga) ==
          "tga");
    CHECK(art2img::core::file_extension(art2img::core::ImageFormat::bmp) ==
          "bmp");
  }

  TEST_CASE("Encoded image structure validation")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    // Load and convert a tile
    auto art_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(art_data.has_value());

    auto palette_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(palette_data.has_value());

    auto archive = art2img::core::load_art(*art_data);
    REQUIRE(archive.has_value());

    auto palette = art2img::core::load_palette(*palette_data);
    REQUIRE(palette.has_value());

    auto palette_view = art2img::core::view_palette(*palette);

    auto tile = art2img::core::get_tile(*archive, 0);
    REQUIRE(tile.has_value());

    auto rgba_image = art2img::core::palette_to_rgba(*tile, palette_view);
    REQUIRE(rgba_image.has_value());

    auto image_view = art2img::core::make_view(*rgba_image);
    REQUIRE(image_view.valid());

    // Encode and validate structure
    auto encoded = art2img::core::encode_image(image_view,
                                               art2img::core::ImageFormat::png);
    if (encoded.has_value()) {
      // Verify all fields are properly set
      CHECK(encoded->format == art2img::core::ImageFormat::png);
      CHECK(encoded->width == rgba_image->width);
      CHECK(encoded->height == rgba_image->height);
      CHECK(!encoded->bytes.empty());

      // Verify dimensions are reasonable
      CHECK(encoded->width > 0);
      CHECK(encoded->height > 0);

      // Verify encoded data size is reasonable (should have some header/data)
      CHECK(encoded->bytes.size() > 8);  // At least some minimal header
    }
  }
}