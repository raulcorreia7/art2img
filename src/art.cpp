/// @file art.cpp
/// @brief Implementation of ART file loading and parsing for Build engine ART files
///
/// This module implements the ART file loading functionality as specified in Architecture §4.3 and §9.
/// It handles:
/// - Loading ART files from filesystem or memory spans
/// - Parsing ART headers, metadata, and pixel data
/// - Building TileView objects referencing contiguous pixel/remap buffers
/// - Optional palette hint logic and sidecar file discovery
/// - Safety checks and validation according to Architecture §14
///
/// The ART format (Build engine):
/// - Header: version (4 bytes), numtiles (4 bytes), localtilestart (4 bytes), localtileend (4 bytes)
/// - Tile arrays: tilesizx[] (2 bytes each), tilesizy[] (2 bytes each), picanm[] (4 bytes each)
/// - Pixel data: column-major palette indices for each tile
/// - Optional remap data: contiguous remap tables for all tiles
///
/// All functions use std::expected<T, Error> for error handling with proper validation
/// according to Architecture §14 validation rules.

#include <art2img/types.hpp>
#include <art2img/art.hpp>
#include <fstream>
#include <algorithm>
#include <cstring>
#include <filesystem>

namespace art2img
{
    using types::byte;
    using types::u8;
    using types::u16;
    using types::u32;

    namespace
    {

        /// @brief Read a 16-bit little-endian value from a byte span
        u16 read_u16_le(std::span<const byte> data, std::size_t offset)
        {
            if (offset + 1 >= data.size())
            {
                return 0;
            }
            return static_cast<u16>(static_cast<u8>(data[offset])) |
                   (static_cast<u16>(static_cast<u8>(data[offset + 1])) << 8);
        }

        /// @brief Read a 32-bit little-endian value from a byte span
        u32 read_u32_le(std::span<const byte> data, std::size_t offset)
        {
            if (offset + 3 >= data.size())
            {
                return 0;
            }
            return static_cast<u32>(static_cast<u8>(data[offset])) |
                   (static_cast<u32>(static_cast<u8>(data[offset + 1])) << 8) |
                   (static_cast<u32>(static_cast<u8>(data[offset + 2])) << 16) |
                   (static_cast<u32>(static_cast<u8>(data[offset + 3])) << 24);
        }

        /// @brief Validate tile dimensions are within reasonable bounds
        constexpr bool is_valid_tile_dimensions(u16 width, u16 height) noexcept
        {
            return (width == 0 && height == 0) || // Allow empty tiles
                   (width > 0 && height > 0 &&
                    width <= constants::MAX_TILE_WIDTH &&
                    height <= constants::MAX_TILE_HEIGHT);
        }

        /// @brief Calculate expected pixel data size for all tiles
        std::size_t calculate_total_pixel_size(
            std::span<const u16> widths,
            std::span<const u16> heights)
        {
            std::size_t total = 0;
            for (std::size_t i = 0; i < widths.size(); ++i)
            {
                total += static_cast<std::size_t>(widths[i]) * static_cast<std::size_t>(heights[i]);
            }
            return total;
        }

        /// @brief Validate ART header consistency
        bool validate_header_consistency(
            u32 version,
            u32 numtiles,
            u32 localtilestart,
            u32 localtileend)
        {
            // Version should be 1 for Build engine ART files
            if (version != 1)
            {
                return false;
            }

            // Tile ranges should be consistent
            if (localtilestart > localtileend)
            {
                return false;
            }

            // Calculate actual tile count from range
            const u32 actual_tile_count = localtileend - localtilestart + 1;

            // numtiles field is unused but should be reasonable
            if (numtiles > 10000 || actual_tile_count > 10000)
            {
                return false;
            }

            return true;
        }

    } // anonymous namespace

    // ============================================================================
    // TileAnimation Implementation
    // ============================================================================

    TileAnimation::TileAnimation(u32 picanm)
        : frame_count(static_cast<u8>(picanm & 0x3F)),
          type(static_cast<Type>((picanm >> 6) & 0x03)),
          speed(static_cast<u8>((picanm >> 24) & 0x0F)),
          x_center_offset(static_cast<std::int8_t>((picanm >> 16) & 0xFF)),
          y_center_offset(static_cast<std::int8_t>((picanm >> 8) & 0xFF))
    {
    }

    u32 TileAnimation::to_picanm() const noexcept
    {
        u32 result = 0;
        result |= static_cast<u32>(frame_count & 0x3F);
        result |= static_cast<u32>(static_cast<u8>(type) & 0x03) << 6;
        result |= static_cast<u32>(static_cast<u8>(y_center_offset) & 0xFF) << 8;
        result |= static_cast<u32>(static_cast<u8>(x_center_offset) & 0xFF) << 16;
        result |= static_cast<u32>(speed & 0x0F) << 24;
        return result;
    }

    // ============================================================================
    // ArtData Implementation
    // ============================================================================

    std::optional<TileView> ArtData::get_tile(std::size_t index) const noexcept
    {
        if (index >= tiles.size())
        {
            return std::nullopt;
        }
        return tiles[index];
    }

    std::optional<TileView> ArtData::get_tile_by_id(u32 tile_id) const noexcept
    {
        // Find the tile with the matching ID
        for (std::size_t i = 0; i < tile_ids.size(); ++i)
        {
            if (tile_ids[i] == tile_id)
            {
                return tiles[i];
            }
        }
        return std::nullopt;
    }

    // ============================================================================
    // ART Loading Implementation
    // ============================================================================

    std::expected<ArtData, Error> load_art_bundle(
        const std::filesystem::path &path,
        PaletteHint hint)
    {
        // Open file in binary mode
        std::ifstream file(path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            return make_error_expected<ArtData>(errc::io_failure,
                                                "Failed to open ART file: " + path.string());
        }

        // Get file size
        const auto file_size = file.tellg();
        if (file_size < 0)
        {
            return make_error_expected<ArtData>(errc::io_failure,
                                                "Failed to determine ART file size: " + path.string());
        }

        // Seek back to beginning
        file.seekg(0, std::ios::beg);
        if (!file)
        {
            return make_error_expected<ArtData>(errc::io_failure,
                                                "Failed to seek in ART file: " + path.string());
        }

        // Read entire file into buffer
        std::vector<byte> buffer(static_cast<std::size_t>(file_size));
        if (!file.read(reinterpret_cast<char *>(buffer.data()), file_size))
        {
            return make_error_expected<ArtData>(errc::io_failure,
                                                "Failed to read ART file: " + path.string());
        }

        // Parse the loaded data
        return load_art_bundle(buffer, hint);
    }

    std::expected<ArtData, Error> load_art_bundle(
        std::span<const byte> data,
        PaletteHint hint)
    {
        ArtData art_data;

        // Minimum ART file size: header (16 bytes) + at least one tile
        constexpr std::size_t MIN_SIZE = 16 + 6 + 6 + 4; // header + 1 tile arrays
        if (data.size() < MIN_SIZE)
        {
            return make_error_expected<ArtData>(errc::invalid_art,
                                                "ART file too small: " + std::to_string(data.size()) +
                                                    " bytes, expected at least " + std::to_string(MIN_SIZE) + " bytes");
        }

        // Parse header
        std::size_t offset = 0;
        art_data.version = read_u32_le(data, offset);
        offset += 4;

        const u32 numtiles = read_u32_le(data, offset);
        offset += 4;

        art_data.tile_start = read_u32_le(data, offset);
        offset += 4;

        art_data.tile_end = read_u32_le(data, offset);
        offset += 4;

        // Validate header consistency
        if (!validate_header_consistency(art_data.version, numtiles, art_data.tile_start, art_data.tile_end))
        {
            return make_error_expected<ArtData>(errc::invalid_art,
                                                "Invalid ART header: version=" + std::to_string(art_data.version) +
                                                    ", tile_range=" + std::to_string(art_data.tile_start) + "-" + std::to_string(art_data.tile_end));
        }

        // Calculate actual tile count
        const u32 tile_count = art_data.tile_end - art_data.tile_start + 1;

        // Check if we have enough data for tile arrays
        const std::size_t arrays_size = tile_count * 2 + tile_count * 2 + tile_count * 4; // sizx + sizy + picanm
        const std::size_t header_with_arrays_size = 16 + arrays_size;
        if (data.size() < header_with_arrays_size)
        {
            return make_error_expected<ArtData>(errc::invalid_art,
                                                "ART file too small for tile arrays: " + std::to_string(data.size()) +
                                                    " bytes, need at least " + std::to_string(header_with_arrays_size) + " bytes");
        }

        // Parse tile arrays
        std::vector<u16> tile_widths(tile_count);
        std::vector<u16> tile_heights(tile_count);
        std::vector<u32> tile_picanms(tile_count);

        // Read widths
        for (u32 i = 0; i < tile_count; ++i)
        {
            tile_widths[i] = read_u16_le(data, offset);
            offset += 2;
        }

        // Read heights
        for (u32 i = 0; i < tile_count; ++i)
        {
            tile_heights[i] = read_u16_le(data, offset);
            offset += 2;
        }

        // Read picanm values
        for (u32 i = 0; i < tile_count; ++i)
        {
            tile_picanms[i] = read_u32_le(data, offset);
            offset += 4;
        }

        // Validate tile dimensions
        for (u32 i = 0; i < tile_count; ++i)
        {
            if (!is_valid_tile_dimensions(tile_widths[i], tile_heights[i]))
            {
                return make_error_expected<ArtData>(errc::invalid_art,
                                                    "Invalid tile dimensions for tile " + std::to_string(i) +
                                                        ": " + std::to_string(tile_widths[i]) + "x" + std::to_string(tile_heights[i]));
            }
        }

        // Calculate total pixel data size
        const std::size_t total_pixel_size = calculate_total_pixel_size(tile_widths, tile_heights);

        // Check if we have enough data for pixel data
        const std::size_t expected_size = header_with_arrays_size + total_pixel_size;
        if (data.size() < expected_size)
        {
            return make_error_expected<ArtData>(errc::invalid_art,
                                                "ART file too small for pixel data: " + std::to_string(data.size()) +
                                                    " bytes, need at least " + std::to_string(expected_size) + " bytes");
        }

        // Allocate and copy pixel data
        art_data.pixels.resize(total_pixel_size);
        std::memcpy(art_data.pixels.data(), data.data() + offset, total_pixel_size);
        offset += total_pixel_size;

        // Load remap data if hint requires it and we have remaining data
        if ((hint == PaletteHint::lookup || hint == PaletteHint::both) && offset < data.size())
        {
            const std::size_t remaining_size = data.size() - offset;
            art_data.remaps.resize(remaining_size);
            std::memcpy(art_data.remaps.data(), data.data() + offset, remaining_size);
        }

        // Build tile views
        std::size_t pixel_offset = 0;
        std::size_t remap_offset = 0;

        for (u32 i = 0; i < tile_count; ++i)
        {
            TileView view;
            view.width = tile_widths[i];
            view.height = tile_heights[i];
            view.animation = TileAnimation(tile_picanms[i]);

            const std::size_t tile_pixel_count = static_cast<std::size_t>(view.width) * static_cast<std::size_t>(view.height);

            // Handle empty tiles
            if (view.width == 0 || view.height == 0)
            {
                view.pixels = types::u8_span{}; // Empty span
                view.remap = types::u8_span{};  // Empty remap span
            }
            else
            {
                // Set pixel data span
                if (pixel_offset + tile_pixel_count <= art_data.pixels.size())
                {
                    view.pixels = types::u8_span(
                        art_data.pixels.data() + pixel_offset,
                        tile_pixel_count);
                }

                // Set remap data span if available
                if (!art_data.remaps.empty() && remap_offset < art_data.remaps.size())
                {
                    // Assume each tile gets a remap table (256 bytes) if we have remap data
                    const std::size_t remap_size = std::min<std::size_t>(256, art_data.remaps.size() - remap_offset);
                    if (remap_size > 0)
                    {
                        view.remap = types::u8_span(
                            art_data.remaps.data() + remap_offset,
                            remap_size);
                        remap_offset += remap_size;
                    }
                }
            }

            // Validate tile view consistency
            if (!view.has_valid_pixel_data())
            {
                return make_error_expected<ArtData>(errc::invalid_art,
                                                    "Inconsistent pixel data for tile " + std::to_string(i) +
                                                        ": expected " + std::to_string(tile_pixel_count) +
                                                        " pixels, got " + std::to_string(view.pixels.size()));
            }

            art_data.tiles.push_back(view);
            art_data.tile_ids.push_back(art_data.tile_start + i);
            pixel_offset += tile_pixel_count;
        }

        return art_data;
    }

    // ============================================================================
    // Helper Functions Implementation
    // ============================================================================

    std::optional<TileView> make_tile_view(const ArtData &art_data, std::size_t index)
    {
        return art_data.get_tile(index);
    }

    std::optional<TileView> make_tile_view_by_id(const ArtData &art_data, u32 tile_id)
    {
        return art_data.get_tile_by_id(tile_id);
    }

    std::filesystem::path discover_sidecar_palette(const std::filesystem::path &art_path)
    {
        // Try to find PALETTE.DAT in the same directory
        const std::filesystem::path palette_path = art_path.parent_path() / "PALETTE.DAT";
        if (std::filesystem::exists(palette_path))
        {
            return palette_path;
        }

        // Try to find a palette with the same basename as the ART file
        const std::filesystem::path same_basename = art_path.parent_path() / art_path.stem().concat(".DAT");
        if (std::filesystem::exists(same_basename))
        {
            return same_basename;
        }

        return {}; // Not found
    }

    std::filesystem::path discover_lookup_file(const std::filesystem::path &art_path)
    {
        // Try to find LOOKUP.DAT in the same directory
        const std::filesystem::path lookup_path = art_path.parent_path() / "LOOKUP.DAT";
        if (std::filesystem::exists(lookup_path))
        {
            return lookup_path;
        }

        return {}; // Not found
    }

    std::expected<std::vector<u8>, Error> load_lookup_data(
        const std::filesystem::path &lookup_path)
    {
        // Open file in binary mode
        std::ifstream file(lookup_path, std::ios::binary | std::ios::ate);
        if (!file)
        {
            return make_error_expected<std::vector<u8>>(errc::io_failure,
                                                        "Failed to open LOOKUP.DAT file: " + lookup_path.string());
        }

        // Get file size
        const auto file_size = file.tellg();
        if (file_size < 0)
        {
            return make_error_expected<std::vector<u8>>(errc::io_failure,
                                                        "Failed to determine LOOKUP.DAT file size: " + lookup_path.string());
        }

        // Seek back to beginning
        file.seekg(0, std::ios::beg);
        if (!file)
        {
            return make_error_expected<std::vector<u8>>(errc::io_failure,
                                                        "Failed to seek in LOOKUP.DAT file: " + lookup_path.string());
        }

        // Validate file size (LOOKUP.DAT should have reasonable size)
        if (file_size < 256)
        {
            return make_error_expected<std::vector<u8>>(errc::invalid_art,
                                                        "Invalid LOOKUP.DAT file size: " + std::to_string(file_size) +
                                                            " bytes (must be at least 256)");
        }

        // Read entire file into buffer
        std::vector<u8> buffer(static_cast<std::size_t>(file_size));
        if (!file.read(reinterpret_cast<char *>(buffer.data()), file_size))
        {
            return make_error_expected<std::vector<u8>>(errc::io_failure,
                                                        "Failed to read LOOKUP.DAT file: " + lookup_path.string());
        }

        return buffer;
    }

    std::expected<std::vector<u8>, Error> load_lookup_data(
        std::span<const byte> data)
    {
        // Validate data size (LOOKUP.DAT should have reasonable size)
        if (data.size() < 256)
        {
            return make_error_expected<std::vector<u8>>(errc::invalid_art,
                                                        "Invalid LOOKUP.DAT data size: " + std::to_string(data.size()) +
                                                            " bytes (must be at least 256)");
        }

        // Convert byte span to uint8 vector
        std::vector<u8> result(data.size());
        for (std::size_t i = 0; i < data.size(); ++i)
        {
            result[i] = static_cast<u8>(data[i]);
        }

        return result;
    }

} // namespace art2img
