/// @file animation.cpp
/// @brief Implementation of ART animation data export functionality
///
/// This module implements animation data extraction from ART files and INI file generation
/// based on the legacy art2tga.c implementation by Mathieu Olivier.
///
/// The animation export functionality:
/// - Extracts animation metadata from ART tile data
/// - Generates INI files with animation sequences and properties
/// - Supports all animation types (none, oscillation, forward, backward)
/// - Compatible with legacy art2tga output format

#include <art2img/types.hpp>
#include <art2img/art.hpp>
#include <art2img/error.hpp>
#include <fstream>
#include <filesystem>
#include <iomanip>

namespace art2img {

using types::u32;
using types::i32;

std::string get_animation_type_string(TileAnimation::Type type)
{
    switch (type)
    {
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

std::expected<std::monostate, Error> export_animation_data(
    const ArtData& art_data,
    const AnimationExportConfig& config)
{
    // Create output directory if it doesn't exist
    std::error_code ec;
    std::filesystem::create_directories(config.output_dir, ec);
    if (ec)
    {
        return make_error_expected<std::monostate>(errc::io_failure,
                                               "Failed to create output directory: " + config.output_dir.string());
    }

    // Open INI file for writing
    std::filesystem::path ini_path = config.output_dir / config.ini_filename;
    std::ofstream ini_file(ini_path);
    if (!ini_file)
    {
        return make_error_expected<std::monostate>(errc::io_failure,
                                               "Failed to create INI file: " + ini_path.string());
    }

    // Write INI header (matches legacy format exactly)
    ini_file << "; This file contains animation data from ART tiles\n";
    ini_file << "; Extracted by art2img v2.0\n\n";

    bool found_animated_tiles = false;

    // Process each tile
    for (std::size_t i = 0; i < art_data.tile_count(); ++i)
    {
        const auto& tile = art_data.tiles[i];
        const auto& tile_id = art_data.tile_ids[i];
        const auto& anim = tile.animation;

        // Skip empty tiles
        if (!tile.is_valid())
        {
            continue;
        }

        // Check if tile has animation data (following legacy logic exactly)
        // Legacy logic: check if any animation bits are set
        u32 picanm = anim.to_picanm();
        bool has_animation_data = ((picanm & 0x3F) != 0) ||           // frame count
                                 (((picanm >> 6) & 0x03) != 0) ||    // animation type  
                                 (((picanm >> 24) & 0x0F) != 0);      // animation speed

        // Write animation sequence if tile has animation data
        if (has_animation_data)
        {
            found_animated_tiles = true;
            
            // Write animation sequence entry (matches legacy format exactly)
            u32 end_tile_id = tile_id + (picanm & 0x3F);
            ini_file << "[tile" << std::setw(4) << std::setfill('0') 
                     << tile_id << ".tga -> tile" << std::setw(4) << std::setfill('0') 
                     << end_tile_id << ".tga]\n";
            
            // Write animation properties
            ini_file << "   AnimationType=" << get_animation_type_string(anim.type) << "\n";
            ini_file << "   AnimationSpeed=" << static_cast<u32>(anim.speed) << "\n";
            ini_file << "\n";
        }

        // Write individual tile data if requested
        if (config.include_non_animated)
        {
            ini_file << "[tile" << std::setw(4) << std::setfill('0') 
                     << tile_id << ".tga]\n";
            ini_file << "   XCenterOffset=" << static_cast<i32>(anim.x_center_offset) << "\n";
            ini_file << "   YCenterOffset=" << static_cast<i32>(anim.y_center_offset) << "\n";
            ini_file << "   OtherFlags=" << (picanm >> 28) << "\n"; // Legacy compatibility
            ini_file << "\n";
        }
    }

    ini_file.close();

    if (!found_animated_tiles)
    {
        return make_error_expected<std::monostate>(errc::no_animation,
                                               "No animated tiles found in ART file");
    }

    return std::monostate{};
}

} // namespace art2img