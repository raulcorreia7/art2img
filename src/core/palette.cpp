#include <art2img/core/palette.hpp>

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>

namespace art2img::core {
namespace {
constexpr std::size_t kPaletteBytes = palette_component_count;
constexpr std::size_t kShadeCountBytes = 2;

}  // namespace

std::expected<Palette, Error> load_palette(
    std::span<const std::byte> blob) noexcept {
  if (blob.size() < kPaletteBytes + kShadeCountBytes) {
    return std::unexpected(make_error(errc::invalid_palette,
                                      "palette data too small"));
  }

  Palette palette{};
  std::memcpy(palette.rgb.data(), blob.data(), kPaletteBytes);

  std::size_t offset = kPaletteBytes;
  std::uint16_t shade_tables = 0;
  std::memcpy(&shade_tables, blob.data() + offset, sizeof(shade_tables));
  palette.shade_table_count = shade_tables;
  offset += kShadeCountBytes;

  if (palette.shade_table_count > palette_color_count) {
    return std::unexpected(
        make_error(errc::invalid_palette, "invalid shade table count"));
  }

  const std::size_t shade_bytes = static_cast<std::size_t>(palette.shade_table_count) *
                                  shade_table_size;
  if (shade_bytes > 0) {
    if (blob.size() < offset + shade_bytes) {
      return std::unexpected(make_error(errc::invalid_palette,
                                        "palette missing shade data"));
    }
    palette.shade_tables.resize(shade_bytes);
    std::memcpy(palette.shade_tables.data(), blob.data() + offset, shade_bytes);
    offset += shade_bytes;
  }

  if (blob.size() >= offset + translucent_table_size) {
    std::memcpy(palette.translucent.data(), blob.data() + offset,
                translucent_table_size);
  } else {
    palette.translucent.fill(0);
  }

  return palette;
}

PaletteView view_palette(const Palette& palette) noexcept {
  PaletteView view{};
  view.rgb = palette.rgb;
  view.shade_table_count = palette.shade_table_count;
  view.shade_tables = palette.shade_tables;
  view.translucent = palette.translucent;
  return view;
}

}  // namespace art2img::core
