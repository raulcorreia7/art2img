#include <art2img/palette/detail/palette_color.hpp>

namespace art2img::palette::detail {

namespace {
constexpr bool is_valid_palette_index(types::u8 index) noexcept {
  return index < constants::PALETTE_SIZE;
}

constexpr types::u8 scale_6bit_to_8bit(types::u8 value) noexcept {
  return static_cast<types::u8>((value * 255 + 31) / 63);
}
} // namespace

color::Color make_palette_color(const Palette &palette,
                                types::u8 index) noexcept {
  if (!is_valid_palette_index(index)) {
    return color::constants::BLACK;
  }

  const std::size_t base =
      static_cast<std::size_t>(index) * constants::COLOR_COMPONENTS;

  return color::make_rgba(scale_6bit_to_8bit(palette.data[base]),
                          scale_6bit_to_8bit(palette.data[base + 1]),
                          scale_6bit_to_8bit(palette.data[base + 2]));
}

} // namespace art2img::palette::detail
