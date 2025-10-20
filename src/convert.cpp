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

#include <art2img/convert.hpp>
#include <art2img/color_helpers.hpp>
#include <art2img/convert/detail/pixel_converter.hpp>
#include <art2img/types.hpp>
#include <algorithm>
#include <cstring>
#include <limits>

namespace art2img
{

    namespace
    {
        using types::u16;
        using types::u8;

        /// @brief Validate coordinates are within tile bounds
        constexpr bool is_valid_coordinates(const TileView &tile, u16 x, u16 y) noexcept
        {
            return x < tile.width && y < tile.height;
        }
        /// @brief Write RGBA value to destination buffer (RGBA format)
        void write_rgba(mutable_u8_span dest, std::size_t offset, const color::Color &color) noexcept
        {
            if (offset + constants::RGBA_BYTES_PER_PIXEL <= dest.size())
            {
                color::write_rgba(dest.data() + offset, color);
            }
        }

        /// @brief For pixels that are fully transparent, set them to neutral color to prevent halo effects
        void clean_transparent_pixels(std::vector<u8> &rgba_data, u16 width, u16 height)
        {
            for (u16 y = 0; y < height; ++y)
            {
                for (u16 x = 0; x < width; ++x)
                {
                    const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
                    if (idx + 3 < rgba_data.size() && rgba_data[idx + 3] == 0)
                    { // Fully transparent pixel
                        // Set RGB to black to prevent magenta bleeding/halo effects
                        rgba_data[idx + 0] = 0; // R
                        rgba_data[idx + 1] = 0; // G
                        rgba_data[idx + 2] = 0; // B
                    }
                }
            }
        }

        /// @brief Apply matte hygiene (erosion + blur) to remove halo effects from alpha channel
        void apply_matte_hygiene(std::vector<u8> &rgba_data, u16 width, u16 height)
        {
            // Extract alpha channel
            std::vector<u8> alpha(static_cast<std::size_t>(width) * height);
            for (u16 y = 0; y < height; ++y)
            {
                for (u16 x = 0; x < width; ++x)
                {
                    const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
                    const std::size_t alpha_idx = static_cast<std::size_t>(y) * width + x;
                    if (idx + 3 < rgba_data.size())
                    {
                        alpha[alpha_idx] = rgba_data[idx + 3];
                    }
                    else
                    {
                        alpha[alpha_idx] = 0;
                    }
                }
            }

            // Erode alpha channel (remove edge pixels)
            std::vector<u8> eroded = alpha;
            if (width > 2 && height > 2)
            {
                for (u16 y = 1; y < height - 1; ++y)
                {
                    for (u16 x = 1; x < width - 1; ++x)
                    {
                        const std::size_t idx = static_cast<std::size_t>(y) * width + x;
                        if (alpha[idx] > 0)
                        {
                            u8 min_neighbor = 255;
                            min_neighbor = std::min(min_neighbor, alpha[idx - width]);     // top
                            min_neighbor = std::min(min_neighbor, alpha[idx + width]);     // bottom
                            min_neighbor = std::min(min_neighbor, alpha[idx - 1]);         // left
                            min_neighbor = std::min(min_neighbor, alpha[idx + 1]);         // right
                            eroded[idx] = min_neighbor;
                        }
                    }
                }
            }

            // Blur eroded alpha (3x3 box blur)
            std::vector<u8> blurred = eroded;
            if (width > 2 && height > 2)
            {
                for (u16 y = 1; y < height - 1; ++y)
                {
                    for (u16 x = 1; x < width - 1; ++x)
                    {
                        const std::size_t idx = static_cast<std::size_t>(y) * width + x;
                        u32 sum = 0;
                        for (types::i16 dy = -1; dy <= 1; ++dy)
                        {
                            for (types::i16 dx = -1; dx <= 1; ++dx)
                            {
                                sum += eroded[idx + static_cast<std::size_t>(dy) * width + dx];
                            }
                        }
                        blurred[idx] = static_cast<u8>(sum / 9);
                    }
                }
            }

            // Apply processed alpha back to RGBA data
            for (u16 y = 0; y < height; ++y)
            {
                for (u16 x = 0; x < width; ++x)
                {
                    const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
                    const std::size_t alpha_idx = static_cast<std::size_t>(y) * width + x;
                    if (idx + 3 < rgba_data.size())
                    {
                        rgba_data[idx + 3] = blurred[alpha_idx];
                    }
                }
            }
        }

    } // anonymous namespace

    // ============================================================================
    // ColumnMajorRowRange Implementation
    // ============================================================================

    ColumnMajorRowRange::iterator::iterator(const TileView &tile, std::span<u8> scratch)
        : tile_(&tile), scratch_(scratch), current_row_(0), max_rows_(tile.height)
    {
    }

    ColumnMajorRowRange::iterator::iterator(u16 current_row, u16 max_rows)
        : current_row_(current_row), max_rows_(max_rows)
    {
    }

    auto ColumnMajorRowRange::iterator::operator*() const -> value_type
    {
        if (!tile_ || current_row_ >= max_rows_ || !tile_->is_valid() || scratch_.size() < tile_->width)
        {
            return {};
        }

        auto row_span = scratch_.first(tile_->width);

        // Extract row from column-major data
        for (u16 x = 0; x < tile_->width; ++x)
        {
            const auto pixel_result = get_pixel_column_major(*tile_, x, current_row_);
            if (pixel_result)
            {
                row_span[x] = pixel_result.value();
            }
            else
            {
                row_span[x] = 0; // Default to black on error
            }
        }

        return value_type(row_span.data(), row_span.size());
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
        const bool lhs_at_end = current_row_ >= max_rows_;
        const bool rhs_at_end = other.current_row_ >= other.max_rows_;
        if (lhs_at_end && rhs_at_end)
        {
            return true;
        }
        return current_row_ == other.current_row_ && max_rows_ == other.max_rows_;
    }

    bool ColumnMajorRowRange::iterator::operator!=(const iterator &other) const noexcept
    {
        return !(*this == other);
    }

    ColumnMajorRowRange::ColumnMajorRowRange(const TileView &tile, std::span<u8> scratch)
        : tile_(&tile), scratch_(scratch)
    {
    }

    auto ColumnMajorRowRange::begin() const -> iterator
    {
        if (!is_valid())
        {
            return iterator(0, 0);
        }
        return iterator(*tile_, scratch_);
    }

    auto ColumnMajorRowRange::end() const -> iterator
    {
        if (!tile_ || scratch_.size() < tile_->width)
        {
            return iterator(0, 0);
        }

        iterator result(*tile_, scratch_);
        result.current_row_ = tile_->height;
        result.max_rows_ = tile_->height;
        return result;
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

        convert::detail::PixelConverter converter{palette, options, tile.remap};

        // Convert each pixel
        for (u16 y = 0; y < tile.height; ++y)
        {
            for (u16 x = 0; x < tile.width; ++x)
            {
                // Sample pixel from column-major data
                const auto pixel_result = get_pixel_column_major(tile, x, y);
                if (!pixel_result)
                {
                    return make_error_expected<Image>(pixel_result.error());
                }

                // Convert pixel through pipeline
                const color::Color rgba = converter(pixel_result.value());

                // Write to destination (row-major)
                const std::size_t dest_offset = static_cast<std::size_t>(y) * result.stride +
                                                static_cast<std::size_t>(x) * constants::RGBA_BYTES_PER_PIXEL;
                write_rgba(result.data, dest_offset, rgba);
            }
        }

        // Apply transparency processing pipeline
        if (options.fix_transparency)
        {
            // Clean transparent pixels to prevent halo effects
            clean_transparent_pixels(result.data, result.width, result.height);
        }

        // Apply matte hygiene for advanced halo removal
        if (options.matte_hygiene)
        {
            apply_matte_hygiene(result.data, result.width, result.height);
            
            // Re-apply premultiplication after matte hygiene if needed
            if (options.premultiply_alpha)
            {
                for (std::size_t i = 0; i < result.data.size(); i += 4)
                {
                    const u8 alpha = result.data[i + 3];
                    if (alpha == 0)
                    {
                        result.data[i + 0] = 0;
                        result.data[i + 1] = 0;
                        result.data[i + 2] = 0;
                    }
                    else if (alpha < 255)
                    {
                        result.data[i + 0] = static_cast<u8>((result.data[i + 0] * alpha + 127) / 255);
                        result.data[i + 1] = static_cast<u8>((result.data[i + 1] * alpha + 127) / 255);
                        result.data[i + 2] = static_cast<u8>((result.data[i + 2] * alpha + 127) / 255);
                    }
                }
            }
        }

        return result;
    }

    ImageView image_view(const Image &image)
    {
        return ImageView(image);
    }

    std::expected<std::monostate, Error> convert_column_to_row_major(
        const TileView &tile,
        std::span<u8> destination)
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
                const auto pixel_result = get_pixel_column_major(tile, x, y);
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

    std::expected<u8, Error> get_pixel_column_major(
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

    ColumnMajorRowRange make_column_major_row_iterator(
        const TileView &tile,
        std::span<u8> scratch)
    {
        return ColumnMajorRowRange(tile, scratch);
    }

} // namespace art2img
