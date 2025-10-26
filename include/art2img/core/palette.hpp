#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <span>
#include <vector>

#include "error.hpp"

namespace art2img::core {

inline constexpr std::size_t palette_color_count = 256;
inline constexpr std::size_t palette_component_count = palette_color_count * 3;
inline constexpr std::size_t shade_table_size = palette_color_count;
inline constexpr std::size_t translucent_table_size = 65536;

struct PaletteView {
  std::span<const std::uint8_t> rgb{};
  std::span<const std::uint8_t> shade_tables{};
  std::span<const std::uint8_t> translucent{};
  std::uint16_t shade_table_count = 0;

  constexpr bool has_shades() const noexcept {
    return shade_table_count > 0 &&
           shade_tables.size() >=
               static_cast<std::size_t>(shade_table_count) * shade_table_size;
  }
};

struct Palette {
  std::array<std::uint8_t, palette_component_count> rgb{};
  std::uint16_t shade_table_count = 0;
  std::vector<std::uint8_t> shade_tables{};
  std::array<std::uint8_t, translucent_table_size> translucent{};

 private:
  friend std::expected<Palette, Error> load_palette(
      std::span<const std::byte>) noexcept;
  friend PaletteView view_palette(const Palette&) noexcept;
};

std::expected<Palette, Error> load_palette(
    std::span<const std::byte> blob) noexcept;

PaletteView view_palette(const Palette&) noexcept;

}  // namespace art2img::core
