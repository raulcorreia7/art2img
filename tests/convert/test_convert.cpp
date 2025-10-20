/// @file test_convert.cpp
/// @brief Unit tests for the conversion module
///
/// This test suite verifies the functionality of the conversion module as specified
/// in Architecture §4.4 and §9. It covers:
/// - Basic tile to RGBA conversion
/// - Palette remapping, shading, transparency fixing, and alpha premultiplication
/// - Column-major to row-major format conversion
/// - Pixel sampling and row iteration utilities
/// - Bounds checking and error handling
/// - Edge cases and error conditions

#include <art2img/convert.hpp>
#include <art2img/convert/detail/row_range.hpp>
#include <art2img/palette.hpp>
#include <art2img/art.hpp>
#include <art2img/color_helpers.hpp>
#include <doctest/doctest.h>
#include <vector>
#include <array>
#include <filesystem>
#include <chrono>

namespace {

using namespace art2img;

/// @brief Create a simple test palette with known values
Palette create_test_palette() {
    Palette palette;
    
    // Set up basic RGB colors (6-bit values, will be scaled to 8-bit)
    // Index 0: Black (for transparency testing)
    palette.data[0] = 0;   // R
    palette.data[1] = 0;   // G
    palette.data[2] = 0;   // B
    
    // Index 1: Red
    palette.data[3] = 63;  // R (max 6-bit)
    palette.data[4] = 0;   // G
    palette.data[5] = 0;   // B
    
    // Index 2: Green
    palette.data[6] = 0;   // R
    palette.data[7] = 63;  // G (max 6-bit)
    palette.data[8] = 0;   // B
    
    // Index 3: Blue
    palette.data[9] = 0;   // R
    palette.data[10] = 0;  // G
    palette.data[11] = 63;  // B (max 6-bit)
    
    // Index 4: White
    palette.data[12] = 63; // R (max 6-bit)
    palette.data[13] = 63; // G (max 6-bit)
    palette.data[14] = 63; // B (max 6-bit)
    
    // Add a simple shade table (just map each color to the next one)
    // Provide two shade tables: index 0 is identity (no shading), index 1 shifts colors
    palette.shade_table_count = 2;
    palette.shade_tables.resize(2 * 256);
    for (int i = 0; i < 256; ++i) {
        palette.shade_tables[i] = static_cast<std::uint8_t>(i);              // Shade 0: identity
        palette.shade_tables[256 + i] = static_cast<std::uint8_t>((i + 1) % 256); // Shade 1: rotate
    }
    
    return palette;
}

/// @brief Create a simple test tile with known pixel data
TileView create_test_tile(std::uint16_t width, std::uint16_t height) {
    static std::vector<std::uint8_t> pixel_storage;
    static std::vector<std::uint8_t> remap_storage;
    
    pixel_storage.resize(width * height);
    remap_storage.resize(256);
    
    // Fill with a pattern: pixels increase from left to right, top to bottom
    for (std::uint16_t y = 0; y < height; ++y) {
        for (std::uint16_t x = 0; x < width; ++x) {
            // Convert to column-major index
            const std::size_t column_major_index = static_cast<std::size_t>(x) * height + y;
            pixel_storage[column_major_index] = static_cast<std::uint8_t>((x + y) % 5);
        }
    }
    
    // Create a simple remap table (identity)
    for (int i = 0; i < 256; ++i) {
        remap_storage[i] = static_cast<std::uint8_t>(i);
    }
    
    TileView tile;
    tile.width = width;
    tile.height = height;
    tile.pixels = pixel_storage;
    tile.remap = remap_storage;
    
    return tile;
}

/// @brief Create an empty tile for edge case testing
TileView create_empty_tile() {
    TileView tile;
    tile.width = 0;
    tile.height = 0;
    tile.pixels = {};
    tile.remap = {};
    return tile;
}

/// @brief Compare two RGBA buffers for equality
bool compare_rgba_buffers(
    const std::vector<std::uint8_t>& buffer1,
    const std::vector<std::uint8_t>& buffer2,
    std::size_t pixel_count) {
    
    if (buffer1.size() < pixel_count * 4 || buffer2.size() < pixel_count * 4) {
        return false;
    }
    
    for (std::size_t i = 0; i < pixel_count * 4; ++i) {
        if (buffer1[i] != buffer2[i]) {
            return false;
        }
    }
    
    return true;
}

/// @brief Get expected RGBA color for a palette index
std::uint32_t get_expected_rgba(std::uint8_t palette_index) {
    // Scale from 6-bit to 8-bit
    const std::uint8_t scale = 4; // 255 / 63
    
    switch (palette_index) {
        case 0: return color::pack_rgba(0, 0, 0);       // Black (fully opaque)
        case 1: return color::pack_rgba(255, 0, 0);     // Red
        case 2: return color::pack_rgba(0, 255, 0);     // Green
        case 3: return color::pack_rgba(0, 0, 255);     // Blue
        case 4: return color::pack_rgba(255, 255, 255); // White
        default: return color::pack_rgba(0, 0, 0);      // Default to black
    }
}

} // anonymous namespace

// ============================================================================
// BASIC CONVERSION TESTS
// ============================================================================

TEST_CASE("to_rgba basic conversion") {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(3, 2);
    const ConversionOptions options;
    
    const auto result = to_rgba(tile, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    CHECK(image.width == 3);
    CHECK(image.height == 2);
    CHECK(image.stride == 12); // 3 pixels * 4 bytes
    CHECK(image.is_valid());
    
    // Check some expected pixel values
    // Pixel (0,0) should have palette index 0 (black)
    const std::size_t pixel_00_offset = 0 * 12 + 0 * 4;
    CHECK(image.data[pixel_00_offset] == 0);     // R
    CHECK(image.data[pixel_00_offset + 1] == 0); // G
    CHECK(image.data[pixel_00_offset + 2] == 0); // B
    CHECK(image.data[pixel_00_offset + 3] == 255); // A

    // Pixel (1,0) should have palette index 1 (red)
    const std::size_t pixel_10_offset = 0 * 12 + 1 * 4;
    CHECK(image.data[pixel_10_offset] == 255);     // R (red)
    CHECK(image.data[pixel_10_offset + 1] == 0);   // G
    CHECK(image.data[pixel_10_offset + 2] == 0);   // B
    CHECK(image.data[pixel_10_offset + 3] == 255); // A
}

TEST_CASE("to_rgba with transparency fixing") {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(2, 2);
    
    // Modify palette to make index 0 magenta for transparency testing
    auto magenta_palette = palette;
    magenta_palette.data[0] = 63;  // R (max 6-bit = 255 scaled)
    magenta_palette.data[1] = 0;   // G
    magenta_palette.data[2] = 63;  // B (max 6-bit = 255 scaled)
    
    ConversionOptions options;
    options.fix_transparency = true;
    
    const auto result = to_rgba(tile, magenta_palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    // Pixel (0,0) should have palette index 0 (magenta) and be transparent
    const std::size_t pixel_00_offset = 0 * 8 + 0 * 4;
    CHECK(image.data[pixel_00_offset] == 0);     // R (set to black for transparent)
    CHECK(image.data[pixel_00_offset + 1] == 0); // G
    CHECK(image.data[pixel_00_offset + 2] == 0); // B
    CHECK(image.data[pixel_00_offset + 3] == 0); // A (transparent)
    
    // Other pixels should be opaque
    const std::size_t pixel_10_offset = 0 * 8 + 1 * 4;
    CHECK(image.data[pixel_10_offset + 3] == 255); // A (opaque)
}

TEST_CASE("to_rgba with shading") {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(2, 2);
    
    ConversionOptions options;
    options.shade_index = 1; // Use second shade table (first applies shading)
    
    const auto result = to_rgba(tile, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    // With shading, palette index 0 should become 1 (red)
    const std::size_t pixel_00_offset = 0 * 8 + 0 * 4;
    CHECK(image.data[pixel_00_offset] == 255);     // R
    CHECK(image.data[pixel_00_offset + 1] == 0);   // G
    CHECK(image.data[pixel_00_offset + 2] == 0);   // B
    CHECK(image.data[pixel_00_offset + 3] == 255); // A
}

TEST_CASE("to_rgba with premultiplied alpha") {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(2, 2);
    
    ConversionOptions options;
    options.fix_transparency = true;
    options.premultiply_alpha = true;
    
    const auto result = to_rgba(tile, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    // Transparent pixel should remain black with alpha 0
    const std::size_t pixel_00_offset = 0 * 8 + 0 * 4;
    CHECK(image.data[pixel_00_offset] == 0);     // R
    CHECK(image.data[pixel_00_offset + 1] == 0); // G
    CHECK(image.data[pixel_00_offset + 2] == 0); // B
    CHECK(image.data[pixel_00_offset + 3] == 0); // A (transparent)
}

TEST_CASE("to_rgba with remapping") {
    const auto palette = create_test_palette();
    auto tile = create_test_tile(2, 2);
    
    // Modify remap table to map index 0 -> 1, 1 -> 2, etc.
    std::vector<std::uint8_t> custom_remap(256);
    for (int i = 0; i < 256; ++i) {
        custom_remap[i] = static_cast<std::uint8_t>((i + 1) % 256);
    }
    
    // Update tile remap
    static std::vector<std::uint8_t> remap_storage = custom_remap;
    tile.remap = remap_storage;
    
    ConversionOptions options;
    options.apply_lookup = true;
    
    const auto result = to_rgba(tile, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    // Pixel (0,0) should have palette index 0 remapped to 1 (red)
    const std::size_t pixel_00_offset = 0 * 8 + 0 * 4;
    CHECK(image.data[pixel_00_offset] == 255);     // R (red)
    CHECK(image.data[pixel_00_offset + 1] == 0);   // G
    CHECK(image.data[pixel_00_offset + 2] == 0);   // B
    CHECK(image.data[pixel_00_offset + 3] == 255); // A
}

// ============================================================================
// ERROR HANDLING TESTS
// ============================================================================

TEST_CASE("to_rgba with invalid tile") {
    const auto palette = create_test_palette();
    const auto empty_tile = create_empty_tile();
    const ConversionOptions options;
    
    const auto result = to_rgba(empty_tile, palette, options);
    
    REQUIRE(!result);
    CHECK(result.error().code == errc::conversion_failure);
}

TEST_CASE("to_rgba with out-of-bounds coordinates") {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(2, 2);
    
    // Create a tile with invalid pixel data size
    TileView invalid_tile = tile;
    static std::vector<std::uint8_t> too_small_pixels(1);
    invalid_tile.pixels = too_small_pixels;
    
    const ConversionOptions options;
    const auto result = to_rgba(invalid_tile, palette, options);
    
    REQUIRE(!result);
    CHECK(result.error().code == errc::conversion_failure);
}

// ============================================================================
// UTILITY FUNCTION TESTS
// ============================================================================

TEST_CASE("image_view creation") {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(3, 2);
    const ConversionOptions options;
    
    const auto image_result = to_rgba(tile, palette, options);
    REQUIRE(image_result);
    
    const auto& image = image_result.value();
    const auto view = image_view(image);
    
    CHECK(view.width == image.width);
    CHECK(view.height == image.height);
    CHECK(view.stride == image.stride);
    CHECK(view.data.data() == image.data.data());
    CHECK(view.is_valid());
}

TEST_CASE("convert_column_to_row_major") {
    const auto tile = create_test_tile(3, 2);
    
    std::vector<std::uint8_t> destination(6); // 3 * 2 pixels
    
    const auto result = convert_column_to_row_major(tile, destination);
    
    REQUIRE(result);
    
    // Check the conversion: pixel (x,y) should be at index y*width + x
    // Original pattern: (0,0)=0, (1,0)=1, (2,0)=2, (0,1)=1, (1,1)=2, (2,1)=3
    CHECK(destination[0] == 0); // (0,0)
    CHECK(destination[1] == 1); // (1,0)
    CHECK(destination[2] == 2); // (2,0)
    CHECK(destination[3] == 1); // (0,1)
    CHECK(destination[4] == 2); // (1,1)
    CHECK(destination[5] == 3); // (2,1)
}

TEST_CASE("convert_column_to_row_major with insufficient buffer") {
    const auto tile = create_test_tile(3, 2);
    
    std::vector<std::uint8_t> too_small(5); // Need 6 bytes
    
    const auto result = convert_column_to_row_major(tile, too_small);
    
    REQUIRE(!result);
    CHECK(result.error().code == errc::conversion_failure);
}

TEST_CASE("get_pixel_column_major") {
    const auto tile = create_test_tile(3, 2);
    
    // Test valid coordinates
    const auto pixel_00 = get_pixel_column_major(tile, 0, 0);
    REQUIRE(pixel_00);
    CHECK(pixel_00.value() == 0);
    
    const auto pixel_10 = get_pixel_column_major(tile, 1, 0);
    REQUIRE(pixel_10);
    CHECK(pixel_10.value() == 1);
    
    const auto pixel_01 = get_pixel_column_major(tile, 0, 1);
    REQUIRE(pixel_01);
    CHECK(pixel_01.value() == 1);
    
    // Test out-of-bounds coordinates
    const auto pixel_out_x = get_pixel_column_major(tile, 3, 0);
    REQUIRE(!pixel_out_x);
    
    const auto pixel_out_y = get_pixel_column_major(tile, 0, 2);
    REQUIRE(!pixel_out_y);
}

TEST_CASE("make_column_major_row_iterator iteration") {
    const auto tile = create_test_tile(3, 2);
    auto rows_owner = convert::detail::ColumnMajorRowRangeOwner(tile);
    auto& row_range = rows_owner.get();
    REQUIRE(row_range.is_valid());
    
    auto it = row_range.begin();
    auto end = row_range.end();
    
    // First row: (0,0)=0, (1,0)=1, (2,0)=2
    REQUIRE(it != end);
    const auto row0 = *it;
    REQUIRE(row0.size() == 3);
    CHECK(row0[0] == 0);
    CHECK(row0[1] == 1);
    CHECK(row0[2] == 2);
    
    ++it;
    
    // Second row: (0,1)=1, (1,1)=2, (2,1)=3
    REQUIRE(it != end);
    const auto row1 = *it;
    REQUIRE(row1.size() == 3);
    CHECK(row1[0] == 1);
    CHECK(row1[1] == 2);
    CHECK(row1[2] == 3);
    
    ++it;
    CHECK(it == end);
}

// ============================================================================
// EDGE CASE TESTS
// ============================================================================

TEST_CASE("conversion with single pixel tile") {
    const auto palette = create_test_palette();
    
    // Create 1x1 tile
    TileView single_tile;
    single_tile.width = 1;
    single_tile.height = 1;
    static std::vector<std::uint8_t> single_pixel = {2}; // Green
    single_tile.pixels = single_pixel;
    
    const ConversionOptions options;
    const auto result = to_rgba(single_tile, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    CHECK(image.width == 1);
    CHECK(image.height == 1);
    CHECK(image.stride == 4);
    
    // Should be green
    CHECK(image.data[0] == 0);   // R
    CHECK(image.data[1] == 255); // G
    CHECK(image.data[2] == 0);   // B
    CHECK(image.data[3] == 255); // A
}

TEST_CASE("conversion with no remap data") {
    const auto palette = create_test_palette();
    
    TileView tile_no_remap;
    tile_no_remap.width = 2;
    tile_no_remap.height = 2;
    static std::vector<std::uint8_t> pixels = {0, 1, 2, 3};
    tile_no_remap.pixels = pixels;
    tile_no_remap.remap = {}; // Empty remap
    
    ConversionOptions options;
    options.apply_lookup = true; // Should not crash with empty remap
    
    const auto result = to_rgba(tile_no_remap, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    CHECK(image.is_valid());
}

TEST_CASE("conversion pipeline order") {
    const auto palette = create_test_palette();
    
    // Create tile with palette index 0
    TileView tile;
    tile.width = 1;
    tile.height = 1;
    static std::vector<std::uint8_t> pixel = {0};
    tile.pixels = pixel;
    
    // Create remap that maps 0 -> 1
    static std::vector<std::uint8_t> remap(256, 0);
    remap[0] = 1;
    tile.remap = remap;
    
    // Apply all options: remap -> shade -> transparency fix -> premultiply
    ConversionOptions options;
    options.apply_lookup = true;
    options.shade_index = 1; // Use shading table that maps 1 -> 2
    options.fix_transparency = true; // Will not affect index 2
    options.premultiply_alpha = true;
    
    const auto result = to_rgba(tile, palette, options);
    
    REQUIRE(result);
    const auto& image = result.value();
    
    // Final color should be green (index 2) with full opacity
    CHECK(image.data[0] == 0);   // R
    CHECK(image.data[1] == 255); // G
    CHECK(image.data[2] == 0);   // B
    CHECK(image.data[3] == 255); // A
}

// ============================================================================
// PERFORMANCE BENCHMARK (Optional)
// ============================================================================

TEST_CASE("performance sanity check" * doctest::skip()) {
    const auto palette = create_test_palette();
    const auto tile = create_test_tile(64, 64);
    const ConversionOptions options;
    
    // Time the conversion
    const auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100; ++i) {
        const auto result = to_rgba(tile, palette, options);
        REQUIRE(result);
    }
    
    const auto end = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Should complete reasonably fast (adjust threshold as needed)
    CHECK(duration.count() < 1000); // Less than 1 second for 100 conversions
    
    // Print timing information
    MESSAGE("Converted 100x 64x64 tiles in ", duration.count(), " ms");
    MESSAGE("Average time per conversion: ", duration.count() / 100.0, " ms");
}
