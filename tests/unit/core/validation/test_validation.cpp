#include <doctest/doctest.h>
#include <art2img/art.hpp>
#include <art2img/detail/validation.hpp>
#include <art2img/palette.hpp>
#include <art2img/types.hpp>

using namespace art2img;

TEST_SUITE("Validation Utilities") {
  TEST_CASE("is_valid_tile_dimensions validates correctly") {
    // Valid cases
    CHECK(detail::is_valid_tile_dimensions(0, 0) ==
          true);  // Empty tiles allowed
    CHECK(detail::is_valid_tile_dimensions(1, 1) == true);
    CHECK(detail::is_valid_tile_dimensions(constants::MAX_TILE_WIDTH,
                                           constants::MAX_TILE_HEIGHT) == true);

    // Invalid cases
    CHECK(detail::is_valid_tile_dimensions(0, 1) ==
          false);  // Mixed zero/non-zero
    CHECK(detail::is_valid_tile_dimensions(1, 0) ==
          false);  // Mixed zero/non-zero
    CHECK(detail::is_valid_tile_dimensions(constants::MAX_TILE_WIDTH + 1,
                                           100) == false);  // Width too large
    CHECK(
        detail::is_valid_tile_dimensions(100, constants::MAX_TILE_HEIGHT + 1) ==
        false);  // Height too large
  }

  TEST_CASE("is_valid_palette_index validates correctly") {
    // All u8 values are valid palette indices since PALETTE_SIZE is 256
    CHECK(detail::is_valid_palette_index(0) == true);
    CHECK(detail::is_valid_palette_index(255) == true);

    // Note: Since the function accepts u8, it can never receive values >= 256
    // The validation ensures that any u8 value is within the palette bounds
  }

  TEST_CASE("is_valid_shade_index validates correctly") {
    const types::u16 shade_count = 32;

    // Valid cases
    CHECK(detail::is_valid_shade_index(shade_count, 0) == true);
    CHECK(detail::is_valid_shade_index(shade_count, shade_count - 1) == true);

    // Invalid cases
    CHECK(detail::is_valid_shade_index(shade_count, shade_count) == false);
    CHECK(detail::is_valid_shade_index(shade_count, shade_count + 1) == false);
  }

  TEST_CASE("is_valid_coordinates validates correctly") {
    TileView tile;
    tile.width = 100;
    tile.height = 50;

    // Valid cases
    CHECK(detail::is_valid_coordinates(tile, 0, 0) == true);
    CHECK(detail::is_valid_coordinates(tile, 99, 49) == true);
    CHECK(detail::is_valid_coordinates(tile, 50, 25) == true);

    // Invalid cases
    CHECK(detail::is_valid_coordinates(tile, 100, 0) ==
          false);                                               // X at boundary
    CHECK(detail::is_valid_coordinates(tile, 0, 50) == false);  // Y at boundary
    CHECK(detail::is_valid_coordinates(tile, 101, 0) ==
          false);  // X out of bounds
    CHECK(detail::is_valid_coordinates(tile, 0, 51) ==
          false);  // Y out of bounds
  }
}