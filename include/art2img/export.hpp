#pragma once

#include <art2img/art.hpp>
#include <art2img/convert.hpp>
#include <art2img/encode.hpp>
#include <art2img/error.hpp>
#include <cstddef>
#include <filesystem>
#include <vector>

namespace art2img {

/// @brief Options for export operations
struct ExportOptions {
  std::filesystem::path output_dir;      ///< Base output directory
  ImageFormat format = ImageFormat::png; ///< Image format for export
  bool organize_by_format = false;       ///< Create subdirectories per format
  bool organize_by_art_file = false;     ///< Create subdirectories per ART file
  std::string filename_prefix = "tile";  ///< Prefix for generated filenames
  ConversionOptions conversion_options; ///< Options for tile to RGBA conversion

  // Parallel processing options
  bool enable_parallel = true; ///< Enable parallel processing
  std::size_t max_threads = 0; ///< Maximum threads (0 = auto-detect)
};

/// @brief Result of an export operation
struct ExportResult {
  std::size_t total_tiles = 0;    ///< Total number of tiles processed
  std::size_t exported_tiles = 0; ///< Number of successfully exported tiles
  std::vector<std::filesystem::path>
      output_files; ///< List of generated output files
};

/// @brief Export a single tile to an image file
/// @param tile The tile to export
/// @param palette The palette to use for conversion
/// @param options Export configuration
/// @return ExportResult on success, or Error on failure
std::expected<ExportResult, Error> export_tile(const TileView &tile,
                                               const Palette &palette,
                                               const ExportOptions &options);

/// @brief Export all tiles from an ART bundle
/// @param art_data The ART data containing tiles
/// @param palette The palette to use for conversion
/// @param options Export configuration
/// @return ExportResult on success, or Error on failure
std::expected<ExportResult, Error>
export_art_bundle(const ArtData &art_data, const Palette &palette,
                  const ExportOptions &options);

/// @brief Export all tiles from multiple ART files
/// @param art_files List of ART file paths
/// @param palette The palette to use for conversion
/// @param options Export configuration
/// @return ExportResult on success, or Error on failure
std::expected<ExportResult, Error>
export_art_files(const std::vector<std::filesystem::path> &art_files,
                 const Palette &palette, const ExportOptions &options);

} // namespace art2img