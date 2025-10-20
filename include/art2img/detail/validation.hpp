#pragma once

#include <art2img/art.hpp>
#include <art2img/palette.hpp>
#include <art2img/types.hpp>

namespace art2img::detail {

/// @brief Validate tile dimensions are within reasonable bounds
/// @param width Tile width to validate
/// @param height Tile height to validate
/// @return true if dimensions are valid, false otherwise
inline constexpr bool is_valid_tile_dimensions(types::u16 width,
                                               types::u16 height) noexcept {
  return (width == 0 && height == 0) ||  // Allow empty tiles
         (width > 0 && height > 0 && width <= constants::MAX_TILE_WIDTH &&
          height <= constants::MAX_TILE_HEIGHT);
}

/// @brief Validate palette index bounds
/// @param index Palette index to validate
/// @return true if index is valid, false otherwise
inline constexpr bool is_valid_palette_index(types::u8 index) noexcept {
  return index < constants::PALETTE_SIZE;
}

/// @brief Validate shade table index bounds
/// @param shade_count Number of shade tables available
/// @param shade Shade table index to validate
/// @return true if index is valid, false otherwise
inline constexpr bool is_valid_shade_index(types::u16 shade_count,
                                           types::u8 shade) noexcept {
  return shade < shade_count;
}

/// @brief Validate coordinates are within tile bounds
/// @param tile The tile view to check against
/// @param x X coordinate to validate
/// @param y Y coordinate to validate
/// @return true if coordinates are valid, false otherwise
inline constexpr bool is_valid_coordinates(const TileView& tile, types::u16 x,
                                           types::u16 y) noexcept {
  return x < tile.width && y < tile.height;
}

}  // namespace art2img::detail