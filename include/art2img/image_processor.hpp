#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <vector>

#include "palette.hpp"

namespace art2img {

// Lightweight view describing an indexed Build tile and optional lookup table.
struct TileView {
  uint16_t width = 0;
  uint16_t height = 0;
  std::span<const uint8_t> pixels;
  std::span<const uint8_t> lookup;

  [[nodiscard]] constexpr std::size_t pixel_count() const {
    return static_cast<std::size_t>(width) * height;
  }

  [[nodiscard]] constexpr bool is_empty() const { return pixel_count() == 0; }

  [[nodiscard]] constexpr bool has_lookup() const { return !lookup.empty(); }
};

// Behaviour switches applied during indexed-to-RGBA conversion.
struct TileConversionOptions {
  bool enable_alpha = true;
  bool fix_transparency = true;
  bool premultiply_alpha = false;
  bool apply_matte_hygiene = false;
};

[[nodiscard]] std::vector<uint8_t> convert_tile_to_rgba(
    const Palette& palette, const TileView& tile,
    const TileConversionOptions& options = {});

}  // namespace art2img

