#pragma once

#include <art2img/error.hpp>
#include <art2img/types.hpp>
#include <expected>
#include <filesystem>
#include <span>
#include <vector>

namespace art2img {

using types::byte;
using types::u16;
using types::u8;

/// @brief Immutable palette data structure containing RGB colors, shade tables,
/// and translucent map
struct Palette {
  /// @brief Raw palette data - 256 entries × 3 bytes (6-bit RGB stored as raw
  /// bytes)
  std::array<u8, constants::PALETTE_DATA_SIZE> data{};

  /// @brief Number of shade tables in the palette
  std::uint16_t shade_table_count = 0;

  /// @brief Shade table data - shade_table_count × 256 entries mapping original
  /// colors to shaded versions
  std::vector<u8> shade_tables{};

  /// @brief Translucent blend table - 64K entries for blending any two palette
  /// colors Zeroed when absent in the source file
  std::array<u8, constants::TRANSLUCENT_TABLE_SIZE> translucent_map{};

  /// @brief Default constructor
  Palette() = default;

  /// @brief Get a read-only view of the palette data
  constexpr std::span<const u8> palette_data() const noexcept { return data; }

  /// @brief Get a read-only view of the shade tables
  std::span<const u8> shade_data() const noexcept { return shade_tables; }

  /// @brief Get a read-only view of the translucent map
  constexpr std::span<const u8> translucent_data() const noexcept {
    return translucent_map;
  }

  /// @brief Check if shade tables are available
  constexpr bool has_shade_tables() const noexcept {
    return shade_table_count > 0 && !shade_tables.empty();
  }

  /// @brief Check if translucent map is available (non-zero)
  constexpr bool has_translucent_map() const noexcept {
    // Check if the translucent map has any non-zero entries
    for (std::size_t i = 0; i < constants::TRANSLUCENT_TABLE_SIZE; ++i) {
      if (translucent_map[i] != 0) {
        return true;
      }
    }
    return false;
  }
};

/// @brief Load a palette from a file path
/// @param path Path to the PALETTE.DAT file
/// @return Expected Palette on success, Error on failure
std::expected<Palette, Error> load_palette(const std::filesystem::path &path);

/// @brief Load a palette from a byte span
/// @param data Span containing the raw PALETTE.DAT data
/// @return Expected Palette on success, Error on failure
std::expected<Palette, Error> load_palette(std::span<const std::byte> data);

/// @brief Convert a palette entry to 32-bit RGBA format
/// @param palette The palette to read from
/// @param index Palette index (0-255)
/// @return 32-bit RGBA value with 8-bit components (0xFF for alpha)
std::uint32_t palette_entry_to_rgba(const Palette &palette, u8 index);

/// @brief Convert a shaded palette entry to 32-bit RGBA format
/// @param palette The palette to read from
/// @param shade Shade table index (0 to shade_table_count-1)
/// @param index Original palette index (0-255)
/// @return 32-bit RGBA value with 8-bit components (0xFF for alpha)
std::uint32_t palette_shaded_entry_to_rgba(const Palette &palette, u8 shade,
                                           u8 index);

/// @brief Get the RGB components of a palette entry as 8-bit values
/// @param palette The palette to read from
/// @param index Palette index (0-255)
/// @return Tuple of (red, green, blue) components scaled to 8-bit
std::tuple<u8, u8, u8> palette_entry_to_rgb(const Palette &palette, u8 index);

/// @brief Get the RGB components of a shaded palette entry as 8-bit values
/// @param palette The palette to read from
/// @param shade Shade table index (0 to shade_table_count-1)
/// @param index Original palette index (0-255)
/// @return Tuple of (red, green, blue) components scaled to 8-bit
std::tuple<u8, u8, u8> palette_shaded_entry_to_rgb(const Palette &palette,
                                                   u8 shade, u8 index);

/// @brief Convert a palette entry to a Color structure
/// @param palette The palette to read from
/// @param index Palette index (0-255)
/// @return Color structure with RGBA values (palette data is BGR format)
color::Color palette_entry_to_color(const Palette &palette, u8 index);

/// @brief Convert a shaded palette entry to a Color structure
/// @param palette The palette to read from
/// @param shade Shade table index (0 to shade_table_count-1)
/// @param index Original palette index (0-255)
/// @return Color structure with RGBA values (palette data is BGR format)
color::Color palette_shaded_entry_to_color(const Palette &palette, u8 shade,
                                           u8 index);

} // namespace art2img
