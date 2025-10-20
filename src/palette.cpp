/// @file palette.cpp
/// @brief Implementation of palette loading and conversion functions for Duke
/// Nukem 3D PALETTE.DAT files
///
/// This module implements the palette loading functionality as specified in
/// Architecture §4.2 and §9. It handles:
/// - Loading PALETTE.DAT files from filesystem or memory spans
/// - Parsing 6-bit RGB palette data (256 entries × 3 bytes)
/// - Reading shade tables for color shading effects
/// - Processing translucent blend tables for transparency effects
/// - Converting palette entries to 8-bit RGB and 32-bit RGBA formats
/// - Validation of file format and bounds checking
///
/// The PALETTE.DAT format:
/// - Bytes 0-767: 256 RGB palette entries (6-bit per component)
/// - Bytes 768-769: Little-endian 16-bit shade table count
/// - Bytes 770+: Shade tables (count × 256 bytes each)
/// - Final 65536 bytes: Translucent blend table (optional)
///
/// All functions use std::expected<T, Error> for error handling with proper
/// validation according to Architecture §14 validation rules.

#include <algorithm>
#include <cstring>
#include <fstream>
#include <tuple>

#include <art2img/palette.hpp>
#include <art2img/palette/detail/palette_color.hpp>

namespace art2img {
using art2img::types::byte;
using art2img::types::u16;
using art2img::types::u32;
using art2img::types::u8;

namespace {

/// @brief Validate palette index bounds
constexpr bool is_valid_palette_index(u8 index) noexcept {
  return index < constants::PALETTE_SIZE;
}

/// @brief Validate shade table index bounds
constexpr bool is_valid_shade_index(u16 shade_count, u8 shade) noexcept {
  return shade < shade_count;
}

/// @brief Read a 16-bit little-endian value from a byte span
u16 read_u16_le(std::span<const byte> data, std::size_t offset) {
  if (offset + 1 >= data.size()) {
    return 0;
  }
  return static_cast<u16>(static_cast<u8>(data[offset])) |
         (static_cast<u16>(static_cast<u8>(data[offset + 1])) << 8);
}

}  // anonymous namespace

std::expected<Palette, Error> load_palette(const std::filesystem::path& path) {
  // Open file in binary mode
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    return make_error_expected<Palette>(
        errc::io_failure, "Failed to open palette file: " + path.string());
  }

  // Get file size
  const auto file_size = file.tellg();
  if (file_size < 0) {
    return make_error_expected<Palette>(
        errc::io_failure,
        "Failed to determine palette file size: " + path.string());
  }

  // Seek back to beginning
  file.seekg(0, std::ios::beg);
  if (!file) {
    return make_error_expected<Palette>(
        errc::io_failure, "Failed to seek in palette file: " + path.string());
  }

  // Read entire file into buffer
  std::vector<std::byte> buffer(static_cast<std::size_t>(file_size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
    return make_error_expected<Palette>(
        errc::io_failure, "Failed to read palette file: " + path.string());
  }

  // Parse the loaded data
  return load_palette(buffer);
}

std::expected<Palette, Error> load_palette(std::span<const byte> data) {
  Palette palette;

  // Minimum required size: palette data (768 bytes) + shade count (2 bytes)
  constexpr std::size_t MIN_SIZE = constants::PALETTE_DATA_SIZE + 2;
  if (data.size() < MIN_SIZE) {
    return make_error_expected<Palette>(
        errc::invalid_palette,
        "Palette data too small: " + std::to_string(data.size()) +
            " bytes, expected at least " + std::to_string(MIN_SIZE) + " bytes");
  }

  // Copy palette data (768 bytes: 256 entries × 3 RGB components)
  std::memcpy(palette.data.data(), data.data(), constants::PALETTE_DATA_SIZE);

  // Read shade table count (little-endian 16-bit)
  std::size_t offset = constants::PALETTE_DATA_SIZE;
  palette.shade_table_count = read_u16_le(data, offset);
  offset += 2;

  // Validate shade table count
  if (palette.shade_table_count > 256) {  // Reasonable upper limit
    return make_error_expected<Palette>(
        errc::invalid_palette, "Invalid shade table count: " +
                                   std::to_string(palette.shade_table_count));
  }

  // Calculate expected sizes
  const std::size_t shade_tables_size =
      static_cast<std::size_t>(palette.shade_table_count) *
      constants::SHADE_TABLE_SIZE;
  // const std::size_t expected_size = MIN_SIZE + shade_tables_size +
  // constants::TRANSLUCENT_TABLE_SIZE; // Unused but kept for reference

  // Check if we have enough data for shade tables
  if (data.size() < MIN_SIZE + shade_tables_size) {
    return make_error_expected<Palette>(
        errc::invalid_palette,
        "Palette data too small for shade tables: " +
            std::to_string(data.size()) + " bytes, need at least " +
            std::to_string(MIN_SIZE + shade_tables_size) + " bytes");
  }

  // Copy shade table data if present
  if (palette.shade_table_count > 0) {
    palette.shade_tables.resize(shade_tables_size);
    std::memcpy(palette.shade_tables.data(), data.data() + offset,
                shade_tables_size);
    offset += shade_tables_size;
  }

  // Copy translucent table data if present (zero out if missing)
  if (data.size() >= offset + constants::TRANSLUCENT_TABLE_SIZE) {
    std::memcpy(palette.translucent_map.data(), data.data() + offset,
                constants::TRANSLUCENT_TABLE_SIZE);
  } else {
    // Zero out translucent map if not present in file
    palette.translucent_map.fill(0);
  }

  return palette;
}

u32 palette_entry_to_rgba(const Palette& palette, u8 index) {
  return palette::detail::make_palette_color(palette, index)
      .to_packed(color::Format::RGBA);
}

u32 palette_shaded_entry_to_rgba(const Palette& palette, u8 shade, u8 index) {
  return palette_shaded_entry_to_color(palette, shade, index)
      .to_packed(color::Format::RGBA);
}

std::tuple<u8, u8, u8> palette_entry_to_rgb(const Palette& palette, u8 index) {
  const auto color = palette::detail::make_palette_color(palette, index);
  return {color.r, color.g, color.b};
}

std::tuple<u8, u8, u8> palette_shaded_entry_to_rgb(const Palette& palette,
                                                   u8 shade, u8 index) {
  // Validate inputs
  if (!is_valid_palette_index(index)) {
    return {0, 0, 0};  // Black for invalid indices
  }

  if (!is_valid_shade_index(palette.shade_table_count, shade) ||
      !palette.has_shade_tables()) {
    // If shade is invalid or no shade tables, return unshaded color
    return palette_entry_to_rgb(palette, index);
  }

  // Look up the shaded palette index
  const std::size_t shade_offset =
      static_cast<std::size_t>(shade) * constants::SHADE_TABLE_SIZE + index;
  const u8 shaded_index = palette.shade_tables[shade_offset];

  // Convert the shaded palette entry to RGB
  return palette_entry_to_rgb(palette, shaded_index);
}

color::Color palette_entry_to_color(const Palette& palette, u8 index) {
  return palette::detail::make_palette_color(palette, index);
}

color::Color palette_shaded_entry_to_color(const Palette& palette, u8 shade,
                                           u8 index) {
  // Validate inputs
  if (!is_valid_palette_index(index)) {
    return color::constants::BLACK;  // Black for invalid indices
  }

  if (!is_valid_shade_index(palette.shade_table_count, shade) ||
      !palette.has_shade_tables()) {
    // If shade is invalid or no shade tables, return unshaded color
    return palette_entry_to_color(palette, index);
  }

  // Look up the shaded palette index
  const std::size_t shade_offset =
      static_cast<std::size_t>(shade) * constants::SHADE_TABLE_SIZE + index;
  const u8 shaded_index = palette.shade_tables[shade_offset];

  // Convert the shaded palette entry to Color
  return palette::detail::make_palette_color(palette, shaded_index);
}

}  // namespace art2img
