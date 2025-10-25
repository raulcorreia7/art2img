#pragma once

#include <cstdint>
#include <expected>
#include <optional>

#include "art.hpp"
#include "image.hpp"
#include "palette.hpp"

namespace art2img::core {

struct ConversionOptions {
  bool apply_lookup = false;
  std::optional<std::uint8_t> shade_index{};
};

struct PostprocessOptions {
  bool apply_transparency_fix = true;
  bool premultiply_alpha = false;
  bool sanitize_matte = false;
};

std::expected<RgbaImage, Error> palette_to_rgba(
    const TileView& tile, PaletteView palette,
    ConversionOptions options = {});

void postprocess_rgba(RgbaImage& image,
                      PostprocessOptions options = {});

}  // namespace art2img::core
