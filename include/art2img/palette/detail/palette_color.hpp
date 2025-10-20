#pragma once

#include <art2img/color_helpers.hpp>
#include <art2img/palette.hpp>

namespace art2img::palette::detail {

color::Color make_palette_color(const Palette &palette,
                                types::u8 index) noexcept;

} // namespace art2img::palette::detail
