/// @file animation.cpp
/// @brief Implementation of ART animation data export functionality
///
/// This module implements animation data extraction from ART files and keeps
/// the INI file generation based on the legacy art2tga.c implementation by
/// Mathieu Olivier.
///
/// The animation export functionality:
/// - Extracts animation metadata from ART tile data
/// - Generates INI files with animation sequences and properties
/// - Supports all animation types (none, oscillating, forward, backward)
/// - Compatible with legacy art2tga output format
/// - Adapted the ini format to be format aware (png, tga, bmp)

#include <art2img/art.hpp>
#include <art2img/encode.hpp>
#include <art2img/error.hpp>
#include <art2img/types.hpp>
#include <filesystem>
#include <fstream>
#include <iomanip>

namespace art2img {

using types::i32;
using types::u32;

std::string get_animation_type_string(TileAnimation::Type type) {
  switch (type) {
  case TileAnimation::Type::none:
    return "none";
  case TileAnimation::Type::oscillating:
    return "oscillation";
  case TileAnimation::Type::forward:
    return "forward";
  case TileAnimation::Type::backward:
    return "backward";
  default:
    return "unknown";
  }
}

/// @brief Get file extension for the given image format
std::string get_image_extension(ImageFormat format) {
  switch (format) {
  case ImageFormat::png:
    return "png";
  case ImageFormat::tga:
    return "tga";
  case ImageFormat::bmp:
    return "bmp";
  default:
    return "bin";
  }
}

std::expected<std::monostate, Error>
export_animation_data(const ArtData &art_data,
                      const AnimationExportConfig &config) {
  // Create output directory if it doesn't exist
  std::error_code ec;
  std::filesystem::create_directories(config.output_dir, ec);
  if (ec) {
    return make_error_expected<std::monostate>(
        errc::io_failure,
        "Failed to create output directory: " + config.output_dir.string());
  }

  // Open INI file for writing
  std::filesystem::path ini_path = config.output_dir / config.ini_filename;
  std::ofstream ini_file(ini_path);
  if (!ini_file) {
    return make_error_expected<std::monostate>(
        errc::io_failure, "Failed to create INI file: " + ini_path.string());
  }

  // Write INI header (matches legacy format exactly)
  ini_file << "; This file contains animation data from ART file\n";
  ini_file << "; Extracted by art2img\n\n";

  bool found_animated_tiles = false;

  // Process each tile
  for (std::size_t i = 0; i < art_data.tile_count(); ++i) {
    const auto &tile = art_data.tiles[i];
    const auto &tile_id = art_data.tile_ids[i];
    const auto &anim = tile.animation;

    // Skip empty tiles
    if (!tile.is_valid()) {
      continue;
    }

    // Check if tile has animation data (following legacy logic exactly)
    // Legacy logic: check if any animation bits are set
    u32 picanm = anim.to_picanm();
    bool has_animation_data = ((picanm & 0x3F) != 0) ||        // frame count
                              (((picanm >> 6) & 0x03) != 0) || // animation type
                              (((picanm >> 24) & 0x0F) != 0); // animation speed

    // Write animation data if tile has animation data
    if (has_animation_data) {
      found_animated_tiles = true;

      // Write animation section header with frame range (legacy format)
      u32 frame_count = (picanm & 0x3F);
      if (frame_count > 0) {
        ini_file << "[tile" << std::setfill('0') << std::setw(4) << tile_id
                 << "." << get_image_extension(config.image_format)
                 << " -> tile" << std::setfill('0') << std::setw(4)
                 << (tile_id + frame_count) << "."
                 << get_image_extension(config.image_format) << "]\n";
      } else {
        ini_file << "[tile" << std::setfill('0') << std::setw(4) << tile_id
                 << "." << get_image_extension(config.image_format) << "]\n";
      }

      // Write animation parameters (legacy format)
      ini_file << "   AnimationType=" << get_animation_type_string(anim.type)
               << "\n";
      ini_file << "   AnimationSpeed=" << static_cast<u32>(anim.speed) << "\n";
      ini_file << "\n";
    }

    // Write tile data section (always write for tiles with animation data,
    // optionally for others)
    if (has_animation_data || config.include_non_animated) {
      // Write tile section header (legacy format)
      ini_file << "[tile" << std::setfill('0') << std::setw(4) << tile_id << "."
               << get_image_extension(config.image_format) << "]\n";

      // Write tile parameters (legacy format)
      ini_file << "   XCenterOffset=" << static_cast<i32>(anim.x_center_offset)
               << "\n";
      ini_file << "   YCenterOffset=" << static_cast<i32>(anim.y_center_offset)
               << "\n";
      ini_file << "   OtherFlags=" << (picanm >> 28)
               << "\n"; // Upper 4 bits as OtherFlags

      // Include image file reference if format awareness is enabled
      if (config.include_image_references) {
        ini_file << "   ImageFile=" << config.base_name << "_" << tile_id
                 << "_0." << get_image_extension(config.image_format) << "\n";
      }

      ini_file << "\n";
    }
  }

  ini_file.close();

  return std::monostate{};
}

} // namespace art2img