#pragma once

#include <art2img/art.hpp>
#include <art2img/error.hpp>
#include <art2img/palette.hpp>
#include <art2img/types.hpp>
#include <cstddef>
#include <expected>
#include <iterator>
#include <span>
#include <vector>

namespace art2img {

using types::byte;
using types::byte_span;
using types::mutable_byte_span;
using types::mutable_u8_span;
using types::u16;
using types::u32;
using types::u8;
using types::u8_span;

/// @brief Options for converting indexed tiles to RGBA images
struct ConversionOptions {
  /// @brief Apply palette remapping if available
  bool apply_lookup = false;

  /// @brief Fix transparency (make Build Engine magenta fully transparent)
  bool fix_transparency = true;

  /// @brief Premultiply alpha channel
  bool premultiply_alpha = false;

  /// @brief Apply matte hygiene (erosion + blur) to remove halo effects
  bool matte_hygiene = false;

  /// @brief Shade table index to apply (0 = no shading)
  u8 shade_index = 0;

  /// @brief Default constructor
  ConversionOptions() = default;

  /// @brief Constructor with all options
  ConversionOptions(bool lookup, bool transparency, bool premult, bool matte,
                    u8 shade)
      : apply_lookup(lookup), fix_transparency(transparency),
        premultiply_alpha(premult), matte_hygiene(matte), shade_index(shade) {}
};

/// @brief Owning RGBA image container
struct Image {
  /// @brief RGBA pixel data (row-major format)
  std::vector<u8> data;

  /// @brief Image width in pixels
  u16 width = 0;

  /// @brief Image height in pixels
  u16 height = 0;

  /// @brief Number of bytes per row (stride)
  std::size_t stride = 0;

  /// @brief Default constructor
  Image() = default;

  /// @brief Constructor with dimensions
  Image(u16 w, u16 h)
      : width(w), height(h),
        stride(static_cast<std::size_t>(w) * constants::RGBA_BYTES_PER_PIXEL) {
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
  std::span<const u8> pixels() const noexcept { return data; }

  /// @brief Get mutable view of pixel data
  std::span<u8> pixels() noexcept { return data; }
};

/// @brief Non-owning view over an Image
struct ImageView {
  /// @brief RGBA pixel data (row-major format)
  std::span<const u8> data;

  /// @brief Image width in pixels
  u16 width = 0;

  /// @brief Image height in pixels
  u16 height = 0;

  /// @brief Number of bytes per row (stride)
  std::size_t stride = 0;

  /// @brief Default constructor
  ImageView() = default;

  /// @brief Constructor from Image
  explicit ImageView(const Image &image)
      : data(image.data), width(image.width), height(image.height),
        stride(image.stride) {}

  /// @brief Constructor from span and dimensions
  ImageView(std::span<const u8> pixel_data, u16 w, u16 h, std::size_t s)
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
    using value_type = std::span<const u8>;
    using difference_type = std::ptrdiff_t;
    using pointer = const value_type *;
    using reference = const value_type &;
    friend class ColumnMajorRowRange;

    /// @brief Default constructor
    iterator() = default;

    /// @brief Constructor for begin iterator
    iterator(const TileView &tile, std::span<u8> scratch);

    /// @brief Constructor for end iterator
    iterator(u16 current_row, u16 max_rows);

    /// @brief Dereference operator
    value_type operator*() const;

    /// @brief Pre-increment operator
    iterator &operator++();

    /// @brief Post-increment operator
    iterator operator++(int);

    /// @brief Equality comparison
    bool operator==(const iterator &other) const noexcept;

    /// @brief Inequality comparison
    bool operator!=(const iterator &other) const noexcept;

  private:
    const TileView *tile_ = nullptr;
    std::span<u8> scratch_;
    u16 current_row_ = 0;
    u16 max_rows_ = 0;
  };

  /// @brief Default constructor
  ColumnMajorRowRange() = default;

  /// @brief Constructor from tile view and scratch buffer
  ColumnMajorRowRange(const TileView &tile, std::span<u8> scratch);

  /// @brief Get begin iterator
  iterator begin() const;

  /// @brief Get end iterator
  iterator end() const;

  /// @brief Check if range is valid
  constexpr bool is_valid() const noexcept {
    return tile_ && tile_->is_valid() && scratch_.size() >= tile_->width;
  }

private:
  const TileView *tile_ = nullptr;
  std::span<u8> scratch_;
};

// ============================================================================
// ============================================================================

// ============================================================================
// CONVERSION FUNCTIONS
// ============================================================================

/// @brief Convert a tile view to RGBA image and linear row/column
/// @param tile The tile view to convert
/// @param palette The palette to use for color conversion
/// @param options Conversion options
/// @return Expected Image on success, Error on failure
std::expected<Image, Error> to_rgba(const TileView &tile,
                                    const Palette &palette,
                                    const ConversionOptions & = {});

/// @brief Create a non-owning view over an image
/// @param image The image to create a view for
/// @return ImageView over the image
ImageView image_view(const Image &image);

/// @brief Convert column-major pixel data to row-major format
/// @param tile The tile view with column-major data
/// @param destination Destination buffer for row-major data
/// @return Expected success on completion, Error on failure
std::expected<std::monostate, Error>
convert_column_to_row_major(const TileView &tile, std::span<u8> destination);

/// @brief Get pixel value at coordinates from column-major data
/// @param tile The tile view to read from
/// @param x X coordinate (column)
/// @param y Y coordinate (row)
/// @return Expected palette index on success, Error on failure
std::expected<u8, Error> get_pixel_column_major(const TileView &tile, u16 x,
                                                u16 y);

/// @brief Create a row iterator for column-major tile data
/// @param tile The tile view to iterate over
/// @param scratch Scratch buffer for temporary row data
/// @return ColumnMajorRowRange for row iteration
ColumnMajorRowRange make_column_major_row_iterator(const TileView &tile,
                                                   std::span<u8> scratch);

} // namespace art2img
