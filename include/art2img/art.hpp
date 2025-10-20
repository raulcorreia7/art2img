#pragma once

#include <art2img/types.hpp>
#include <art2img/error.hpp>
#include <art2img/palette.hpp>
#include <expected>
#include <filesystem>
#include <span>
#include <vector>
#include <optional>

namespace art2img {



using types::byte;
using types::u8;
using types::i8;
using types::u16;
using types::u32;
using types::u8_span;

/// @brief Palette hint for automatic palette discovery
enum class PaletteHint : u8 {
    /// @brief No palette hint - don't attempt auto-discovery
    none = 0,
    
    /// @brief Try to find sidecar palette file (same directory, same basename)
    sidecar = 1,
    
    /// @brief Try to find LOOKUP.DAT for remap tables
    lookup = 2,
    
    /// @brief Try both sidecar and lookup
    both = 3
};

/// @brief Animation information for a tile
struct TileAnimation {
    /// @brief Number of animation frames (0-63, bits 0-5 of picanm)
    u8 frame_count = 0;
    
    /// @brief Animation type (bits 6-7 of picanm)
    enum class Type : u8 {
        none = 0,        ///< 00 = no animation
        oscillating = 1, ///< 01 = oscillating animation
        forward = 2,     ///< 10 = animate forward
        backward = 3     ///< 11 = animate backward
    } type = Type::none;
    
    /// @brief Animation speed/timing (bits 24-27 of picanm)
    u8 speed = 0;
    
    /// @brief X-center offset (bits 8-15 of picanm, signed)
    i8 x_center_offset = 0;
    
    /// @brief Y-center offset (bits 16-23 of picanm, signed)
    i8 y_center_offset = 0;
    
    /// @brief Default constructor
    TileAnimation() = default;
    
    /// @brief Construct from picanm value
    explicit TileAnimation(u32 picanm);
    
    /// @brief Convert back to picanm format
    u32 to_picanm() const noexcept;
};

/// @brief Non-owning view of tile data
struct TileView {
    /// @brief Tile width in pixels
    u16 width = 0;
    
    /// @brief Tile height in pixels
    u16 height = 0;
    
    /// @brief Column-major pixel data (palette indices)
    u8_span pixels;
    
    /// @brief Optional remap table data (may be empty)
    u8_span remap;
    
    /// @brief Animation information
    TileAnimation animation;
    
    /// @brief Check if tile has valid dimensions
    constexpr bool is_valid() const noexcept {
        return width > 0 && height > 0 && !pixels.empty();
    }
    
    /// @brief Check if tile has remap data
    constexpr bool has_remap() const noexcept {
        return !remap.empty();
    }
    
    /// @brief Get total number of pixels
    constexpr std::size_t pixel_count() const noexcept {
        return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    }
    
    /// @brief Check if pixel data size matches expected dimensions
    constexpr bool has_valid_pixel_data() const noexcept {
        return pixels.size() == pixel_count();
    }
};

/// @brief Owning container for ART bundle data
struct ArtData {
    /// @brief ART file version
    u32 version = 0;
    
    /// @brief First tile index in this file
    u32 tile_start = 0;
    
    /// @brief Last tile index in this file
    u32 tile_end = 0;
    
    /// @brief Owning pixel data buffer (column-major format)
    std::vector<u8> pixels;
    
    /// @brief Owning remap data buffer (contiguous for all tiles)
    std::vector<u8> remaps;
    
    /// @brief Tile views referencing the above buffers
    std::vector<TileView> tiles;
    
    /// @brief Tile IDs corresponding to each view
    std::vector<u32> tile_ids;
    
    /// @brief Default constructor
    ArtData() = default;
    
    /// @brief Get number of tiles
    constexpr std::size_t tile_count() const noexcept {
        return tiles.size();
    }
    
    /// @brief Check if data is valid
    constexpr bool is_valid() const noexcept {
        return !tiles.empty() && tiles.size() == tile_ids.size();
    }
    
    /// @brief Get tile view by index (bounds-checked)
    std::optional<TileView> get_tile(std::size_t index) const noexcept;
    
    /// @brief Get tile view by tile ID (bounds-checked)
    std::optional<TileView> get_tile_by_id(u32 tile_id) const noexcept;
};

/// @brief Load an ART bundle from a file path
/// @param path Path to the ART file
/// @param hint Palette hint for auto-discovery of sidecar files
/// @return Expected ArtData on success, Error on failure
std::expected<ArtData, Error> load_art_bundle(
    const std::filesystem::path& path, 
    PaletteHint hint = PaletteHint::none);

/// @brief Load an ART bundle from a byte span
/// @param data Span containing the raw ART file data
/// @param hint Palette hint for auto-discovery of sidecar files
/// @return Expected ArtData on success, Error on failure
std::expected<ArtData, Error> load_art_bundle(
    std::span<const std::byte> data, 
    PaletteHint hint = PaletteHint::none);

/// @brief Create a tile view from art data by index
/// @param art_data The art data container
/// @param index Tile index (0-based)
/// @return TileView if index is valid, empty optional otherwise
std::optional<TileView> make_tile_view(const ArtData& art_data, std::size_t index);

/// @brief Create a tile view from art data by tile ID
/// @param art_data The art data container
/// @param tile_id The tile ID to look up
/// @return TileView if tile_id is found, empty optional otherwise
std::optional<TileView> make_tile_view_by_id(const ArtData& art_data, u32 tile_id);

/// @brief Discover sidecar palette file for an ART file
/// @param art_path Path to the ART file
/// @return Path to discovered palette file, or empty path if not found
std::filesystem::path discover_sidecar_palette(const std::filesystem::path& art_path);

/// @brief Discover LOOKUP.DAT file for remap tables
/// @param art_path Path to the ART file (used as reference directory)
/// @return Path to LOOKUP.DAT file, or empty path if not found
std::filesystem::path discover_lookup_file(const std::filesystem::path& art_path);

/// @brief Load LOOKUP.DAT remap data
/// @param lookup_path Path to LOOKUP.DAT file
/// @return Expected vector of remap data on success, Error on failure
std::expected<std::vector<types::u8>, Error> load_lookup_data(
    std::span<const types::byte> data);

/// @brief Animation data export configuration
struct AnimationExportConfig {
    /// @brief Output directory for animation files
    std::filesystem::path output_dir = ".";
    
    /// @brief Base filename for animation tiles
    std::string base_name = "tile";
    
    /// @brief Include tiles without animation data
    bool include_non_animated = true;
    
    /// @brief Generate INI file with animation metadata
    bool generate_ini = true;
    
    /// @brief INI filename (default: animdata.ini)
    std::string ini_filename = "animdata.ini";
    
    /// @brief Image format for exported tiles
    ImageFormat image_format = ImageFormat::png;
    
    /// @brief Include image file references in INI output
    bool include_image_references = true;
};

/// @brief Export animation data from ART file to INI format
/// @param art_data The art data container
/// @param config Export configuration
/// @return Expected monostate on success, Error on failure
std::expected<std::monostate, Error> export_animation_data(
    const ArtData& art_data,
    const AnimationExportConfig& config = {});

/// @brief Get animation type string from enum value
/// @param type Animation type enum
/// @return String representation of animation type
std::string get_animation_type_string(TileAnimation::Type type);

} // namespace art2img
