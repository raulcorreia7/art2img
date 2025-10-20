#pragma once

#include <art2img/color_helpers.hpp>
#include <art2img/convert.hpp>
#include <art2img/palette.hpp>

namespace art2img::convert::detail {

struct PixelConverter {
  const Palette &palette;
  const ConversionOptions &options;
  types::u8_span remap;

  color::Color operator()(types::u8 pixel_index) const noexcept;

private:
  types::u8 remap_index(types::u8 index) const noexcept;
  color::Color select_palette_color(types::u8 index) const noexcept;
  color::Color apply_transparency(color::Color color,
                                  types::u8 index) const noexcept;
};

} // namespace art2img::convert::detail
