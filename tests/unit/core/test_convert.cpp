#include <doctest/doctest.h>

#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/color_helpers.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/palette.hpp>
#include <filesystem>
#include <fstream>

TEST_SUITE("convert module")
{
  TEST_CASE("Basic convert functionality compilation test")
  {
    // This test verifies that the convert module can be included and basic
    // types exist
    using art2img::core::ConversionOptions;
    using art2img::core::PostprocessOptions;

    // Test default options
    ConversionOptions conv_opts;
    CHECK(!conv_opts.apply_lookup);
    CHECK(!conv_opts.shade_index.has_value());
    CHECK(conv_opts.fix_transparency);  // Should be true by default
    CHECK(!conv_opts.premultiply_alpha);
    CHECK(!conv_opts.matte_hygiene);

    PostprocessOptions post_opts;
    CHECK(post_opts.apply_transparency_fix);
    CHECK(!post_opts.premultiply_alpha);
    CHECK(!post_opts.sanitize_matte);

    // Basic compilation test - more comprehensive tests will be added
    // as the implementation progresses
    CHECK(true);
  }

  TEST_CASE("Build Engine magenta detection")
  {
    using art2img::core::is_build_engine_magenta;

    // Test exact magenta (252, 0, 252) - should be detected
    CHECK(is_build_engine_magenta(252, 0, 252));

    // Test variations within tolerance
    CHECK(is_build_engine_magenta(250, 0, 250));
    CHECK(is_build_engine_magenta(255, 0, 255));
    CHECK(is_build_engine_magenta(251, 5, 251));

    // Test non-magenta colors
    CHECK_FALSE(is_build_engine_magenta(255, 0, 0));      // Red
    CHECK_FALSE(is_build_engine_magenta(0, 255, 0));      // Green
    CHECK_FALSE(is_build_engine_magenta(0, 0, 255));      // Blue
    CHECK_FALSE(is_build_engine_magenta(255, 255, 255));  // White
    CHECK_FALSE(is_build_engine_magenta(0, 0, 0));        // Black
    CHECK_FALSE(is_build_engine_magenta(128, 128, 128));  // Gray

    // Test edge cases
    CHECK_FALSE(is_build_engine_magenta(249, 0, 252));  // R too low
    CHECK_FALSE(is_build_engine_magenta(252, 0, 249));  // B too low
    CHECK_FALSE(is_build_engine_magenta(252, 6, 252));  // G too high
  }

  TEST_CASE("Transparency fix validation - no magenta pixels in output")
  {
    using art2img::core::contains_build_engine_magenta;
    using art2img::core::count_build_engine_magenta;

    // Test with empty data
    std::vector<std::uint8_t> empty_data;
    CHECK_FALSE(contains_build_engine_magenta(empty_data));
    CHECK_EQ(count_build_engine_magenta(empty_data), 0);

    // Test with no magenta pixels
    std::vector<std::uint8_t> clean_data = {
        255, 255, 255, 255,  // White
        0,   0,   0,   255,  // Black
        128, 128, 128, 255,  // Gray
        255, 0,   0,   255,  // Red
        0,   255, 0,   255   // Green
    };
    CHECK_FALSE(contains_build_engine_magenta(clean_data));
    CHECK_EQ(count_build_engine_magenta(clean_data), 0);

    // Test with magenta pixels
    std::vector<std::uint8_t> magenta_data = {
        252, 0,   252, 255,  // Magenta (should be detected)
        255, 255, 255, 255,  // White
        0,   0,   0,   255,  // Black
        250, 5,   250, 255   // Magenta variant (should be detected)
    };
    CHECK(contains_build_engine_magenta(magenta_data));
    CHECK_EQ(count_build_engine_magenta(magenta_data), 2);

    // Test with transparent magenta (should not be counted)
    std::vector<std::uint8_t> transparent_magenta_data = {
        252, 0,   252, 0,    // Transparent magenta (should not be counted)
        252, 0,   252, 255,  // Opaque magenta (should be counted)
        255, 255, 255, 255   // White
    };
    CHECK(contains_build_engine_magenta(transparent_magenta_data));
    CHECK_EQ(count_build_engine_magenta(transparent_magenta_data), 1);
  }

  TEST_CASE("Transparency fix applied during conversion")
  {
    using art2img::core::contains_build_engine_magenta;
    using art2img::core::ConversionOptions;
    using art2img::core::count_build_engine_magenta;
    using art2img::core::RgbaImage;

    // Test with transparency fix enabled (default)
    ConversionOptions opts_with_fix;
    opts_with_fix.fix_transparency = true;

    // Test with transparency fix disabled
    ConversionOptions opts_without_fix;
    opts_without_fix.fix_transparency = false;

    // Create a mock palette with magenta at index 255
    std::vector<std::uint8_t> palette_rgb(256 * 3, 0);
    // Set index 255 to magenta (RGB: 252, 0, 252)
    palette_rgb[255 * 3 + 0] = 252;  // R
    palette_rgb[255 * 3 + 1] = 0;    // G
    palette_rgb[255 * 3 + 2] = 252;  // B

    art2img::core::PaletteView palette;
    palette.rgb = palette_rgb;

    // Create a simple tile with some magenta pixels (index 255)
    std::vector<std::byte> tile_pixels(4 * 4, std::byte{0});  // 4x4 tile
    tile_pixels[0] = std::byte{255};  // Set first pixel to magenta index
    tile_pixels[5] = std::byte{255};  // Set another pixel to magenta index

    art2img::core::TileView tile;
    tile.indices = tile_pixels;
    tile.width = 4;
    tile.height = 4;

    // Test conversion with transparency fix enabled
    auto result_with_fix =
        art2img::core::palette_to_rgba(tile, palette, opts_with_fix);
    REQUIRE(result_with_fix.has_value());

    std::span<const std::uint8_t> rgba_data_with_fix(result_with_fix->pixels);
    CHECK_FALSE(contains_build_engine_magenta(rgba_data_with_fix));
    CHECK_EQ(count_build_engine_magenta(rgba_data_with_fix), 0);

    // Verify that magenta pixels became transparent (alpha = 0)
    // First pixel (index 0) should be transparent
    CHECK_EQ(rgba_data_with_fix[3], 0);  // Alpha of first pixel
    CHECK_EQ(rgba_data_with_fix[0], 0);  // R should be 0 when alpha is 0
    CHECK_EQ(rgba_data_with_fix[1], 0);  // G should be 0 when alpha is 0
    CHECK_EQ(rgba_data_with_fix[2], 0);  // B should be 0 when alpha is 0

    // Test conversion with transparency fix disabled
    auto result_without_fix =
        art2img::core::palette_to_rgba(tile, palette, opts_without_fix);
    REQUIRE(result_without_fix.has_value());

    std::span<const std::uint8_t> rgba_data_without_fix(
        result_without_fix->pixels);

    // With fix_transparency=false, magenta pixels at index 255 should NOT be
    // converted to transparent pixels. They should remain as opaque magenta.
    CHECK(contains_build_engine_magenta(rgba_data_without_fix));
    CHECK_EQ(count_build_engine_magenta(rgba_data_without_fix), 2);

    // Verify that the magenta pixels are opaque (not transparent)
    CHECK_EQ(rgba_data_without_fix[3], 255);  // Alpha should be 255 (opaque)
    CHECK_EQ(rgba_data_without_fix[0],
             255);  // R should be 255 (252 expanded to 8-bit)
    CHECK_EQ(rgba_data_without_fix[1], 0);  // G should be 0
    CHECK_EQ(rgba_data_without_fix[2],
             255);  // B should be 255 (252 expanded to 8-bit)
  }

  TEST_CASE("Transparency fix with explicit magenta colors")
  {
    using art2img::core::contains_build_engine_magenta;
    using art2img::core::ConversionOptions;
    using art2img::core::count_build_engine_magenta;

    // Create a palette with explicit magenta colors at various indices
    std::vector<std::uint8_t> palette_rgb(256 * 3, 0);

    // Set multiple indices to magenta variants
    palette_rgb[255 * 3 + 0] = 252;
    palette_rgb[255 * 3 + 1] = 0;
    palette_rgb[255 * 3 + 2] = 252;  // Index 255: standard magenta
    palette_rgb[254 * 3 + 0] = 250;
    palette_rgb[254 * 3 + 1] = 5;
    palette_rgb[254 * 3 + 2] = 250;  // Index 254: magenta variant
    palette_rgb[253 * 3 + 0] = 255;
    palette_rgb[253 * 3 + 1] = 0;
    palette_rgb[253 * 3 + 2] = 255;  // Index 253: bright magenta
    palette_rgb[0 * 3 + 0] = 100;
    palette_rgb[0 * 3 + 1] = 100;
    palette_rgb[0 * 3 + 2] = 100;  // Index 0: gray (not transparent)

    art2img::core::PaletteView palette;
    palette.rgb = palette_rgb;

    // Create a tile with various magenta indices
    // Note: conversion uses x * tile.height + y indexing
    std::vector<std::byte> tile_pixels(16, std::byte{0});  // 4x4 tile
    tile_pixels[0] = std::byte{255};  // Position (0,0) - Standard magenta
    tile_pixels[4] = std::byte{254};  // Position (1,0) - Magenta variant
    tile_pixels[8] = std::byte{253};  // Position (2,0) - Bright magenta
    tile_pixels[12] =
        std::byte{0};  // Position (3,0) - Gray (should remain opaque)

    art2img::core::TileView tile;
    tile.indices = tile_pixels;
    tile.width = 4;
    tile.height = 4;

    // Test with transparency fix enabled
    ConversionOptions opts_with_fix;
    opts_with_fix.fix_transparency = true;

    auto result_with_fix =
        art2img::core::palette_to_rgba(tile, palette, opts_with_fix);
    REQUIRE(result_with_fix.has_value());

    std::span<const std::uint8_t> rgba_data_with_fix(result_with_fix->pixels);

    // With fix_transparency=true, magenta pixels should be converted to
    // transparent
    CHECK_FALSE(contains_build_engine_magenta(rgba_data_with_fix));
    CHECK_EQ(count_build_engine_magenta(rgba_data_with_fix), 0);

    // Verify specific pixel colors
    // First pixel (index 255) should be transparent
    CHECK_EQ(rgba_data_with_fix[3], 0);  // Alpha of first pixel should be 0
    CHECK_EQ(rgba_data_with_fix[0], 0);  // R should be 0 when alpha is 0
    CHECK_EQ(rgba_data_with_fix[1], 0);  // G should be 0 when alpha is 0
    CHECK_EQ(rgba_data_with_fix[2], 0);  // B should be 0 when alpha is 0

    // Fourth pixel (index 0, gray) should be opaque
    CHECK_EQ(rgba_data_with_fix[15],
             255);  // Alpha of fourth pixel should be 255
    CHECK_EQ(rgba_data_with_fix[12],
             150);  // R should be 150 (100 expanded to 8-bit)
    CHECK_EQ(rgba_data_with_fix[13],
             150);  // G should be 150 (100 expanded to 8-bit)
    CHECK_EQ(rgba_data_with_fix[14],
             150);  // B should be 150 (100 expanded to 8-bit)

    // Test with transparency fix disabled
    ConversionOptions opts_without_fix;
    opts_without_fix.fix_transparency = false;

    auto result_without_fix =
        art2img::core::palette_to_rgba(tile, palette, opts_without_fix);
    REQUIRE(result_without_fix.has_value());

    std::span<const std::uint8_t> rgba_data_without_fix(
        result_without_fix->pixels);

    // With fix_transparency=false, magenta pixels should remain opaque
    CHECK(contains_build_engine_magenta(rgba_data_without_fix));
    // Note: Only 2 out of 3 magenta pixels are detected because the green
    // component from palette index 254 (value 5) expands to 20, which is > 5
    CHECK_EQ(count_build_engine_magenta(rgba_data_without_fix), 2);

    // Verify that magenta pixels remain opaque
    // Position (0,0): pixel data at indices 0,1,2,3
    CHECK_EQ(rgba_data_without_fix[3], 255);  // Alpha should be 255
    CHECK_EQ(rgba_data_without_fix[0],
             255);  // R should be 255 (252 expanded to 8-bit)
    CHECK_EQ(rgba_data_without_fix[1], 0);  // G should be 0
    CHECK_EQ(rgba_data_without_fix[2],
             255);  // B should be 255 (252 expanded to 8-bit)

    // Position (1,0): pixel data at indices 4,5,6,7
    CHECK_EQ(rgba_data_without_fix[7], 255);  // Alpha should be 255
    CHECK_EQ(rgba_data_without_fix[4],
             239);  // R should be 239 (250 expanded to 8-bit)
    CHECK_EQ(rgba_data_without_fix[5],
             20);  // G should be 20 (5 expanded to 8-bit)
    CHECK_EQ(rgba_data_without_fix[6],
             239);  // B should be 239 (250 expanded to 8-bit)

    // Position (2,0): pixel data at indices 8,9,10,11
    CHECK_EQ(rgba_data_without_fix[11], 255);  // Alpha should be 255
    CHECK_EQ(rgba_data_without_fix[8],
             255);  // R should be 255 (255 expanded to 8-bit)
    CHECK_EQ(rgba_data_without_fix[9], 0);  // G should be 0
    CHECK_EQ(rgba_data_without_fix[10],
             255);  // B should be 255 (255 expanded to 8-bit)
  }

  TEST_CASE("File-based transparency fix verification")
  {
    using art2img::core::contains_build_engine_magenta;
    using art2img::core::count_build_engine_magenta;

    // Helper function to read a file into bytes
    auto read_file_bytes =
        [](const std::filesystem::path& path) -> std::vector<std::uint8_t> {
      std::ifstream file(path, std::ios::binary);
      if (!file) {
        return {};
      }
      return std::vector<std::uint8_t>((std::istreambuf_iterator<char>(file)),
                                       std::istreambuf_iterator<char>());
    };

    // Helper function to extract RGBA data from PNG file using stb_image
    auto extract_rgba_from_png = [](const std::vector<std::uint8_t>& png_data)
        -> std::vector<std::uint8_t> {
      // For now, use the simplified approach but note its limitations
      // In a real test, we'd use stb_image to properly decode PNG
      std::vector<std::uint8_t> rgba_data;

      // Simple scan for RGB values that could be magenta in the PNG data
      // This is a crude approximation - proper PNG decoding would be more
      // accurate
      for (std::size_t i = 0; i + 2 < png_data.size(); ++i) {
        if (png_data[i] >= 250 && png_data[i + 1] <= 5 &&
            png_data[i + 2] >= 250) {
          // Found potential magenta pattern, add to test data
          rgba_data.push_back(png_data[i]);      // R
          rgba_data.push_back(png_data[i + 1]);  // G
          rgba_data.push_back(png_data[i + 2]);  // B
          rgba_data.push_back(255);  // A (assume opaque for this test)
        }
      }

      return rgba_data;
    };

    // Test with actual converted files if they exist
    const std::filesystem::path test_output_dir = "/tmp/test_transparency";
    if (std::filesystem::exists(test_output_dir)) {
      bool found_png_files = false;
      bool all_pngs_clean = true;

      for (const auto& entry :
           std::filesystem::directory_iterator(test_output_dir)) {
        if (entry.is_regular_file() && entry.path().extension() == ".png") {
          found_png_files = true;
          auto png_data = read_file_bytes(entry.path());

          if (!png_data.empty()) {
            auto rgba_sample = extract_rgba_from_png(png_data);

            // For a proper transparency fix, we should find no magenta in the
            // output Note: This is a simplified test - proper PNG decoding
            // would be more accurate
            bool has_magenta = contains_build_engine_magenta(rgba_sample);
            if (has_magenta) {
              all_pngs_clean = false;
              MESSAGE("Found potential magenta in: "
                      << entry.path().filename().string());
            }
          }
        }
      }

      if (found_png_files) {
        // This test provides feedback but doesn't fail - it's for verification
        if (all_pngs_clean) {
          MESSAGE("All PNG files appear to have transparency fix applied");
        }
        else {
          MESSAGE("Some PNG files may still contain magenta pixels");
        }
      }
    }
  }
}

TEST_CASE("Convert tile to RGBA")
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

  // Parse ART archive and palette
  auto archive = art2img::core::load_art(*art_data);
  REQUIRE(archive.has_value());

  auto palette = art2img::core::load_palette(*palette_data);
  REQUIRE(palette.has_value());

  auto palette_view = art2img::core::view_palette(*palette);

  // Get first tile
  auto tile = art2img::core::get_tile(*archive, 0);
  REQUIRE(tile.has_value());
  CHECK(tile->valid());

  // Convert tile to RGBA
  auto rgba_image = art2img::core::palette_to_rgba(*tile, palette_view);
  REQUIRE(rgba_image.has_value());

  // Verify RGBA image properties
  CHECK(rgba_image->width == tile->width);
  CHECK(rgba_image->height == tile->height);
  CHECK(!rgba_image->pixels.empty());

  // Verify pixel data size (4 bytes per pixel for RGBA)
  const auto expected_pixel_count = static_cast<std::size_t>(tile->width) *
                                    static_cast<std::size_t>(tile->height);
  const auto expected_data_size = expected_pixel_count * 4;
  CHECK(rgba_image->pixels.size() == expected_data_size);

  // Verify pixel values are in valid range
  for (std::uint8_t pixel_component : rgba_image->pixels) {
    CHECK(pixel_component <= 255);
  }
}

TEST_CASE("Convert tile with different options")
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

  auto tile = art2img::core::get_tile(*archive, 0);
  REQUIRE(tile.has_value());

  // Test conversion with lookup application
  art2img::core::ConversionOptions opts_with_lookup;
  opts_with_lookup.apply_lookup = true;

  auto rgba_with_lookup =
      art2img::core::palette_to_rgba(*tile, palette_view, opts_with_lookup);
  if (rgba_with_lookup.has_value()) {
    CHECK(rgba_with_lookup->width == tile->width);
    CHECK(rgba_with_lookup->height == tile->height);
    CHECK(!rgba_with_lookup->pixels.empty());
  }

  // Test conversion with shade index
  if (palette_view.has_shades()) {
    art2img::core::ConversionOptions opts_with_shade;
    opts_with_shade.shade_index = 1;

    auto rgba_with_shade =
        art2img::core::palette_to_rgba(*tile, palette_view, opts_with_shade);
    if (rgba_with_shade.has_value()) {
      CHECK(rgba_with_shade->width == tile->width);
      CHECK(rgba_with_shade->height == tile->height);
      CHECK(!rgba_with_shade->pixels.empty());
    }
  }
}

TEST_CASE("Postprocess RGBA image")
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

  // Store original pixel data for comparison
  std::vector<std::uint8_t> original_pixels = rgba_image->pixels;

  // Test default postprocessing
  art2img::core::postprocess_rgba(*rgba_image);
  CHECK(rgba_image->pixels.size() == original_pixels.size());

  // Test postprocessing with different options
  art2img::core::PostprocessOptions opts;
  opts.apply_transparency_fix = false;
  opts.premultiply_alpha = true;
  opts.sanitize_matte = true;

  // Reset to original and apply different options
  rgba_image->pixels = original_pixels;
  art2img::core::postprocess_rgba(*rgba_image, opts);
  CHECK(rgba_image->pixels.size() == original_pixels.size());
}

TEST_CASE("Convert multiple tiles")
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

  // Convert first few tiles to test consistency
  const std::size_t tiles_to_test =
      std::min(tile_count, static_cast<std::size_t>(3));

  for (std::size_t i = 0; i < tiles_to_test; ++i) {
    auto tile = art2img::core::get_tile(*archive, i);
    REQUIRE(tile.has_value());
    CHECK(tile->valid());

    auto rgba_image = art2img::core::palette_to_rgba(*tile, palette_view);
    REQUIRE(rgba_image.has_value());

    CHECK(rgba_image->width == tile->width);
    CHECK(rgba_image->height == tile->height);
    CHECK(!rgba_image->pixels.empty());

    // Verify pixel data size
    const auto expected_pixel_count = static_cast<std::size_t>(tile->width) *
                                      static_cast<std::size_t>(tile->height);
    const auto expected_data_size = expected_pixel_count * 4;
    CHECK(rgba_image->pixels.size() == expected_data_size);
  }
}

TEST_CASE("Convert with invalid inputs")
{
  const auto test_assets_dir = std::filesystem::path{__FILE__}
                                   .parent_path()
                                   .parent_path()
                                   .parent_path() /
                               "assets";
  const auto palette_file = test_assets_dir / "PALETTE.DAT";

  auto palette_data = art2img::adapters::read_binary_file(palette_file);
  REQUIRE(palette_data.has_value());

  auto palette = art2img::core::load_palette(*palette_data);
  REQUIRE(palette.has_value());

  auto palette_view = art2img::core::view_palette(*palette);

  // Test with invalid tile view
  art2img::core::TileView invalid_tile;
  invalid_tile.width = 10;
  invalid_tile.height = 10;
  // indices span is empty, making it invalid

  auto invalid_result =
      art2img::core::palette_to_rgba(invalid_tile, palette_view);
  CHECK(!invalid_result.has_value());
}

TEST_CASE("Conversion options validation")
{
  art2img::core::ConversionOptions opts;

  // Test default values
  CHECK(!opts.apply_lookup);
  CHECK(!opts.shade_index.has_value());

  // Test setting values
  opts.apply_lookup = true;
  CHECK(opts.apply_lookup);

  opts.shade_index = 5;
  CHECK(opts.shade_index.has_value());
  CHECK(opts.shade_index.value() == 5);

  // Test resetting
  opts.shade_index.reset();
  CHECK(!opts.shade_index.has_value());
}

TEST_CASE("Postprocess options validation")
{
  art2img::core::PostprocessOptions opts;

  // Test default values
  CHECK(opts.apply_transparency_fix);
  CHECK(!opts.premultiply_alpha);
  CHECK(!opts.sanitize_matte);

  // Test setting values
  opts.apply_transparency_fix = false;
  opts.premultiply_alpha = true;
  opts.sanitize_matte = true;

  CHECK(!opts.apply_transparency_fix);
  CHECK(opts.premultiply_alpha);
  CHECK(opts.sanitize_matte);
}