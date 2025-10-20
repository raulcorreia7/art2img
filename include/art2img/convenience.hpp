#pragma once

#include <expected>
#include <filesystem>
#include <vector>

#include <art2img/art.hpp>
#include <art2img/export.hpp>
#include <art2img/palette.hpp>

namespace art2img {

/// @brief Convenience function to load an ART file and export all tiles in one call
/// @param art_path Path to the ART file
/// @param palette_path Path to the palette file (optional)
/// @param output_dir Directory to export tiles to
/// @param format Image format for export (default: PNG)
/// @return ExportResult on success, or Error on failure
inline std::expected<ExportResult, Error> convert_and_export(
    const std::filesystem::path& art_path,
    const std::filesystem::path& palette_path,
    const std::filesystem::path& output_dir,
    ImageFormat format = ImageFormat::png) {

  // Load the palette
  auto palette_result = load_palette(palette_path);
  if (!palette_result) {
    return std::unexpected(palette_result.error());
  }
  const auto& palette = palette_result.value();

  // Load the ART file
  auto art_result = load_art_bundle(art_path);
  if (!art_result) {
    return std::unexpected(art_result.error());
  }
  const auto& art_data = art_result.value();

  // Set up export options
  ExportOptions options;
  options.output_dir = output_dir;
  options.format = format;

  // Export the ART bundle
  return export_art_bundle(art_data, palette, options);
}

/// @brief Convenience function to load an ART file with auto-discovered palette and export all tiles
/// @param art_path Path to the ART file
/// @param output_dir Directory to export tiles to
/// @param format Image format for export (default: PNG)
/// @return ExportResult on success, or Error on failure
inline std::expected<ExportResult, Error> convert_and_export(
    const std::filesystem::path& art_path,
    const std::filesystem::path& output_dir,
    ImageFormat format = ImageFormat::png) {

  // Try to discover palette file
  const auto palette_path = discover_sidecar_palette(art_path);
  if (palette_path.empty()) {
    return std::unexpected(
        Error(errc::io_failure,
              "No palette file found for ART file: " + art_path.string()));
  }

  // Load the palette
  auto palette_result = load_palette(palette_path);
  if (!palette_result) {
    return std::unexpected(palette_result.error());
  }
  const auto& palette = palette_result.value();

  // Load the ART file
  auto art_result = load_art_bundle(art_path);
  if (!art_result) {
    return std::unexpected(art_result.error());
  }
  const auto& art_data = art_result.value();

  // Set up export options
  ExportOptions options;
  options.output_dir = output_dir;
  options.format = format;

  // Export the ART bundle
  return export_art_bundle(art_data, palette, options);
}

/// @brief Convenience function to convert a single tile to an image in memory
/// @param art_path Path to the ART file
/// @param palette_path Path to the palette file
/// @param tile_index Index of the tile to convert (0-based)
/// @return Image on success, or Error on failure
inline std::expected<Image, Error> convert_tile(
    const std::filesystem::path& art_path,
    const std::filesystem::path& palette_path, std::size_t tile_index) {

  // Load the palette
  auto palette_result = load_palette(palette_path);
  if (!palette_result) {
    return std::unexpected(palette_result.error());
  }
  const auto& palette = palette_result.value();

  // Load the ART file
  auto art_result = load_art_bundle(art_path);
  if (!art_result) {
    return std::unexpected(art_result.error());
  }
  const auto& art_data = art_result.value();

  // Get the tile view
  auto tile_view = make_tile_view(art_data, tile_index);
  if (!tile_view) {
    return std::unexpected(
        Error(errc::invalid_art,
              "Invalid tile index: " + std::to_string(tile_index)));
  }

  // Convert to RGBA
  return to_rgba(tile_view.value(), palette);
}

}  // namespace art2img