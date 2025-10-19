/// @file convert.cpp
/// @brief Implementation of tile to RGBA conversion functions for Build engine ART files
///
/// This module implements the conversion functionality as specified in Architecture §4.4 and §9.
/// It handles:
/// - Converting indexed tiles to RGBA images
/// - Palette remapping, shading, transparency fixing, and alpha premultiplication
/// - Column-major to row-major format conversion
/// - Pixel sampling and row iteration utilities
/// - Bounds checking and validation according to Architecture §14
///
/// Conversion pipeline follows: remaps -> shade -> palette conversion -> transparency fix -> premultiply
///
/// All functions use std::expected<T, Error> for error handling with proper validation
/// according to Architecture §14 validation rules.

#include <art2img/types.hpp>
#include <art2img/convert.hpp>
#include <algorithm>
#include <cstring>
#include <limits>

using namespace art2img::types;
namespace art2img
{

    namespace
    {

        /// @brief Validate coordinates are within tile bounds
        constexpr bool is_valid_coordinates(const TileView &tile, u16 x, u16 y) noexcept
        {
            return x < tile.width && y < tile.height;
        }

        /// @brief Apply palette remapping to a pixel index
        u8 apply_remap(u8 pixel, u8_span remap) noexcept
        {
            if (remap.empty() || pixel >= remap.size())
            {
                return pixel; // No remapping available or out of bounds
            }
            return remap[pixel];
        }

        /// @brief Fix transparency for palette index 0
        u32 fix_transparency(u32 rgba, u8 pixel_index, bool should_fix) noexcept
        {
            if (!should_fix || pixel_index != 0)
            {
                return rgba;
            }
            // Make index 0 fully transparent (keep RGB but set alpha to 0)
            return rgba & 0x00FFFFFF; // Clear alpha byte
        }

        /// @brief Premultiply alpha channel
        u32 premultiply_alpha(u32 rgba) noexcept
        {
            const u8 a = static_cast<u8>((rgba >> 24) & 0xFF);
            const u8 r = static_cast<u8>((rgba >> 16) & 0xFF);
            const u8 g = static_cast<u8>((rgba >> 8) & 0xFF);
            const u8 b = static_cast<u8>(rgba & 0xFF);

            const u16 alpha_factor = static_cast<u16>(a) + 1; // +1 for rounding
            const u8 r_premult = static_cast<u8>((static_cast<u16>(r) * alpha_factor) >> 8);
            const u8 g_premult = static_cast<u8>((static_cast<u16>(g) * alpha_factor) >> 8);
            const u8 b_premult = static_cast<u8>((static_cast<u16>(b) * alpha_factor) >> 8);

            return (static_cast<u32>(a) << 24) |
                   (static_cast<u32>(r_premult) << 16) |
                   (static_cast<u32>(g_premult) << 8) |
                   static_cast<u32>(b_premult);
        }

        /// @brief Convert a single pixel through the full conversion pipeline
        u32 convert_pixel(
            u8 pixel_index,
            const Palette &palette,
            const ConversionOptions &options,
            u8_span remap)
        {

            // Step 1: Apply remapping if requested and available
            if (options.apply_lookup && !remap.empty())
            {
                pixel_index = apply_remap(pixel_index, remap);
            }

            // Step 2: Get color from palette (handles BGR->RGB conversion internally)
            color::Color color;
            if (palette.shade_table_count > 0)
            {
                color = palette_shaded_entry_to_color(palette, options.shade_index, pixel_index);
            }
            else
            {
                color = palette_entry_to_color(palette, pixel_index);
            }

            // Step 3: Fix transparency if requested
            if (options.fix_transparency && pixel_index == 0)
            {
                color = color.make_transparent();
            }

            // Step 4: Premultiply alpha if requested
            if (options.premultiply_alpha)
            {
                color = color.premultiplied();
            }

            // Step 5: Convert to RGBA format for output
            return color.to_packed(color::Format::RGBA);
        }
        /// @brief Write RGBA value to destination buffer (RGBA format)
        void write_rgba(mutable_u8_span dest, std::size_t offset, u32 rgba) noexcept
        {
            if (offset + 3 < dest.size())
            {
                dest[offset] = static_cast<u8>((rgba >> 16) & 0xFF);     // Red
                dest[offset + 1] = static_cast<u8>((rgba >> 8) & 0xFF);  // Green
                dest[offset + 2] = static_cast<u8>(rgba & 0xFF);         // Blue
                dest[offset + 3] = static_cast<u8>((rgba >> 24) & 0xFF); // Alpha
            }
        }

    } // anonymous namespace

    // ============================================================================
    // ColumnMajorRowRange Implementation
    // ============================================================================

    ColumnMajorRowRange::iterator::iterator(const TileView &tile, mutable_u8_span scratch)
        : tile_(&tile), scratch_(scratch), current_row_(0), max_rows_(tile.height)
    {
        if (tile.is_valid())
        {
            row_buffer_.resize(tile.width);
        }
    }

    ColumnMajorRowRange::iterator::iterator(u16 current_row, u16 max_rows)
        : current_row_(current_row), max_rows_(max_rows)
    {
    }

    auto ColumnMajorRowRange::iterator::operator*() const -> value_type
    {
        if (!tile_ || current_row_ >= max_rows_ || !tile_->is_valid())
        {
            return {};
        }

        // Extract row from column-major data
        for (u16 x = 0; x < tile_->width; ++x)
        {
            const auto pixel_result = sample_column_major_index(*tile_, x, current_row_);
            if (pixel_result)
            {
                row_buffer_[x] = pixel_result.value();
            }
            else
            {
                row_buffer_[x] = 0; // Default to black on error
            }
        }

        return row_buffer_;
    }

    auto ColumnMajorRowRange::iterator::operator++() -> iterator &
    {
        ++current_row_;
        return *this;
    }

    auto ColumnMajorRowRange::iterator::operator++(int) -> iterator
    {
        iterator temp = *this;
        ++current_row_;
        return temp;
    }

    bool ColumnMajorRowRange::iterator::operator==(const iterator &other) const noexcept
    {
        return current_row_ == other.current_row_ && max_rows_ == other.max_rows_;
    }

    bool ColumnMajorRowRange::iterator::operator!=(const iterator &other) const noexcept
    {
        return !(*this == other);
    }

    ColumnMajorRowRange::ColumnMajorRowRange(const TileView &tile, mutable_u8_span scratch)
        : tile_(&tile), scratch_(scratch)
    {
    }

    auto ColumnMajorRowRange::begin() const -> iterator
    {
        return iterator(*tile_, scratch_);
    }

    auto ColumnMajorRowRange::end() const -> iterator
    {
        return iterator(tile_->height, tile_->height);
    }

    // ============================================================================
    // CONVERSION FUNCTIONS IMPLEMENTATION
    // ============================================================================

    std::expected<Image, Error> to_rgba(
        const TileView &tile,
        const Palette &palette,
        const ConversionOptions &options)
    {

        // Validate inputs
        if (!tile.is_valid())
        {
            return make_error_expected<Image>(errc::conversion_failure,
                                              "Invalid tile view: empty or invalid dimensions");
        }

        // Create output image
        Image result(tile.width, tile.height);
        if (!result.is_valid())
        {
            return make_error_expected<Image>(errc::conversion_failure,
                                              "Failed to create output image with dimensions " +
                                                  std::to_string(tile.width) + "x" + std::to_string(tile.height));
        }

        // Convert each pixel
        for (u16 y = 0; y < tile.height; ++y)
        {
            for (u16 x = 0; x < tile.width; ++x)
            {
                // Sample pixel from column-major data
                const auto pixel_result = sample_column_major_index(tile, x, y);
                if (!pixel_result)
                {
                    return make_error_expected<Image>(pixel_result.error());
                }

                // Convert pixel through pipeline
                const u32 rgba = convert_pixel(
                    pixel_result.value(), palette, options, tile.remap);

                // Write to destination (row-major)
                const std::size_t dest_offset = static_cast<std::size_t>(y) * result.stride +
                                                static_cast<std::size_t>(x) * constants::RGBA_BYTES_PER_PIXEL;
                write_rgba(result.data, dest_offset, rgba);
            }
        }

        return result;
    }

    ImageView image_view(const Image &image)
    {
        return ImageView(image);
    }

    std::expected<std::monostate, Error> copy_column_major_to_row_major(
        const TileView &tile,
        mutable_u8_span destination)
    {

        // Validate inputs
        if (!tile.is_valid())
        {
            return make_error_expected(errc::conversion_failure,
                                       "Invalid tile view: empty or invalid dimensions");
        }

        const std::size_t required_size = static_cast<std::size_t>(tile.width) *
                                          static_cast<std::size_t>(tile.height);
        if (destination.size() < required_size)
        {
            return make_error_expected(errc::conversion_failure,
                                       "Destination buffer too small: need " + std::to_string(required_size) +
                                           " bytes, got " + std::to_string(destination.size()));
        }

        // Convert from column-major to row-major
        for (u16 y = 0; y < tile.height; ++y)
        {
            for (u16 x = 0; x < tile.width; ++x)
            {
                const auto pixel_result = sample_column_major_index(tile, x, y);
                if (!pixel_result)
                {
                    return make_error_expected(pixel_result.error());
                }

                // Write to row-major position
                const std::size_t dest_index = static_cast<std::size_t>(y) * tile.width +
                                               static_cast<std::size_t>(x);
                destination[dest_index] = pixel_result.value();
            }
        }

        return make_success();
    }

    std::expected<u8, Error> sample_column_major_index(
        const TileView &tile,
        u16 x,
        u16 y)
    {

        // Validate coordinates
        if (!is_valid_coordinates(tile, x, y))
        {
            return make_error_expected<u8>(errc::conversion_failure,
                                           "Pixel coordinates out of bounds: (" + std::to_string(x) +
                                               "," + std::to_string(y) + ") for tile size " +
                                               std::to_string(tile.width) + "x" + std::to_string(tile.height));
        }

        // Validate pixel data
        if (tile.pixels.empty())
        {
            return make_error_expected<u8>(errc::conversion_failure,
                                           "Tile has no pixel data");
        }

        // Convert from column-major to linear index
        // Column-major format: all pixels for column 0, then column 1, etc.
        const std::size_t linear_index = static_cast<std::size_t>(x) * tile.height +
                                         static_cast<std::size_t>(y);

        if (linear_index >= tile.pixels.size())
        {
            return make_error_expected<u8>(errc::conversion_failure,
                                           "Pixel index out of range: " + std::to_string(linear_index) +
                                               " >= " + std::to_string(tile.pixels.size()));
        }

        return tile.pixels[linear_index];
    }

    ColumnMajorRowRange column_major_rows(
        const TileView &tile,
        mutable_u8_span scratch)
    {
        return ColumnMajorRowRange(tile, scratch);
    }

} // namespace art2img