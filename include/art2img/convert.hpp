#pragma once

#include <art2img/types.hpp>
#include <art2img/error.hpp>
#include <art2img/palette.hpp>
#include <art2img/art.hpp>
#include <expected>
#include <span>
#include <vector>
#include <iterator>
#include <cstddef>

namespace art2img {

/// @brief Options for converting indexed tiles to RGBA images
struct ConversionOptions {
    /// @brief Apply palette remapping if available
    bool apply_lookup = false;
    
    /// @brief Fix transparency (make index 0 fully transparent)
    bool fix_transparency = false;
    
    /// @brief Premultiply alpha channel
    bool premultiply_alpha = false;
    
    /// @brief Shade table index to apply (0 = no shading)
    std::uint8_t shade_index = 0;
    
    /// @brief Default constructor
    ConversionOptions() = default;
    
    /// @brief Constructor with all options
    ConversionOptions(bool lookup, bool transparency, bool premult, std::uint8_t shade)
        : apply_lookup(lookup), fix_transparency(transparency), 
          premultiply_alpha(premult), shade_index(shade) {}
};

/// @brief Owning RGBA image container
struct Image {
    /// @brief RGBA pixel data (row-major format)
    std::vector<std::uint8_t> data;
    
    /// @brief Image width in pixels
    std::uint16_t width = 0;
    
    /// @brief Image height in pixels
    std::uint16_t height = 0;
    
    /// @brief Number of bytes per row (stride)
    std::size_t stride = 0;
    
    /// @brief Default constructor
    Image() = default;
    
    /// @brief Constructor with dimensions
    Image(std::uint16_t w, std::uint16_t h)
        : width(w), height(h), stride(static_cast<std::size_t>(w) * constants::RGBA_BYTES_PER_PIXEL) {
        data.resize(stride * static_cast<std::size_t>(h));
    }
    
    /// @brief Check if image is valid
    constexpr bool is_valid() const noexcept {
        return width > 0 && height > 0 && stride > 0 && 
               data.size() == stride * static_cast<std::size_t>(height);
    }
    
    /// @brief Get total number of pixels
    constexpr std::size_t pixel_count() const noexcept {
        return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    }
    
    /// @brief Get read-only view of pixel data
    std::span<const std::uint8_t> pixels() const noexcept {
        return data;
    }
    
    /// @brief Get mutable view of pixel data
    std::span<std::uint8_t> pixels() noexcept {
        return data;
    }
};

/// @brief Non-owning view over an Image
struct ImageView {
    /// @brief RGBA pixel data (row-major format)
    std::span<const std::uint8_t> data;
    
    /// @brief Image width in pixels
    std::uint16_t width = 0;
    
    /// @brief Image height in pixels
    std::uint16_t height = 0;
    
    /// @brief Number of bytes per row (stride)
    std::size_t stride = 0;
    
    /// @brief Default constructor
    ImageView() = default;
    
    /// @brief Constructor from Image
    explicit ImageView(const Image& image)
        : data(image.data), width(image.width), height(image.height), stride(image.stride) {}
    
    /// @brief Constructor from span and dimensions
    ImageView(std::span<const std::uint8_t> pixel_data, std::uint16_t w, std::uint16_t h, std::size_t s)
        : data(pixel_data), width(w), height(h), stride(s) {}
    
    /// @brief Check if view is valid
    constexpr bool is_valid() const noexcept {
        return width > 0 && height > 0 && stride > 0 && 
               data.size() >= stride * static_cast<std::size_t>(height);
    }
    
    /// @brief Get total number of pixels
    constexpr std::size_t pixel_count() const noexcept {
        return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    }
};

/// @brief Range object for iterating over column-major data by rows
class ColumnMajorRowRange {
public:
    /// @brief Iterator for row data
    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = std::span<const std::uint8_t>;
        using difference_type = std::ptrdiff_t;
        using pointer = const value_type*;
        using reference = const value_type&;
        
        /// @brief Default constructor
        iterator() = default;
        
        /// @brief Constructor for begin iterator
        iterator(const TileView& tile, std::span<std::uint8_t> scratch);
        
        /// @brief Constructor for end iterator
        iterator(std::uint16_t current_row, std::uint16_t max_rows);
        
        /// @brief Dereference operator
        value_type operator*() const;
        
        /// @brief Pre-increment operator
        iterator& operator++();
        
        /// @brief Post-increment operator
        iterator operator++(int);
        
        /// @brief Equality comparison
        bool operator==(const iterator& other) const noexcept;
        
        /// @brief Inequality comparison
        bool operator!=(const iterator& other) const noexcept;
        
    private:
        const TileView* tile_ = nullptr;
        std::span<std::uint8_t> scratch_;
        std::uint16_t current_row_ = 0;
        std::uint16_t max_rows_ = 0;
        mutable std::vector<std::uint8_t> row_buffer_;
    };
    
    /// @brief Default constructor
    ColumnMajorRowRange() = default;
    
    /// @brief Constructor from tile view and scratch buffer
    ColumnMajorRowRange(const TileView& tile, std::span<std::uint8_t> scratch);
    
    /// @brief Get begin iterator
    iterator begin() const;
    
    /// @brief Get end iterator
    iterator end() const;
    
    /// @brief Check if range is valid
    constexpr bool is_valid() const noexcept {
        return tile_ && tile_->is_valid() && !scratch_.empty();
    }
    
private:
    const TileView* tile_ = nullptr;
    std::span<std::uint8_t> scratch_;
};

// ============================================================================
// CONVERSION FUNCTIONS
// ============================================================================

/// @brief Convert a tile view to RGBA image
/// @param tile The tile view to convert
/// @param palette The palette to use for color conversion
/// @param options Conversion options
/// @return Expected Image on success, Error on failure
std::expected<Image, Error> to_rgba(
    const TileView& tile, 
    const Palette& palette, 
    const ConversionOptions& options = {});

/// @brief Create a non-owning view over an image
/// @param image The image to create a view for
/// @return ImageView over the image
ImageView image_view(const Image& image);

/// @brief Copy column-major pixel data to row-major format
/// @param tile The tile view with column-major data
/// @param destination Destination buffer for row-major data
/// @return Expected success on completion, Error on failure
std::expected<std::monostate, Error> copy_column_major_to_row_major(
    const TileView& tile, 
    std::span<std::uint8_t> destination);

/// @brief Sample a pixel index from column-major data
/// @param tile The tile view to sample from
/// @param x X coordinate (column)
/// @param y Y coordinate (row)
/// @return Expected palette index on success, Error on failure
std::expected<std::uint8_t, Error> sample_column_major_index(
    const TileView& tile, 
    std::uint16_t x, 
    std::uint16_t y);

/// @brief Create a range for iterating over tile rows
/// @param tile The tile view to iterate over
/// @param scratch Scratch buffer for temporary row data
/// @return ColumnMajorRowRange for row iteration
ColumnMajorRowRange column_major_rows(
    const TileView& tile, 
    std::span<std::uint8_t> scratch);

} // namespace art2img