#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace art2img {

/// @brief Core constants for palette and image processing
namespace constants {

/// @brief Number of colors in a standard palette
constexpr std::size_t PALETTE_SIZE = 256;

/// @brief Number of color components (RGB) per palette entry
constexpr std::size_t COLOR_COMPONENTS = 3;

/// @brief Total bytes in base palette data (256 entries × 3 components)
constexpr std::size_t PALETTE_DATA_SIZE = PALETTE_SIZE * COLOR_COMPONENTS;

/// @brief Number of bits per color component in legacy palette data
constexpr std::size_t PALETTE_BITS_PER_COMPONENT = 6;

/// @brief Maximum value for a 6-bit color component
constexpr std::uint8_t PALETTE_COMPONENT_MAX =
    (1 << PALETTE_BITS_PER_COMPONENT) - 1;  // 63

/// @brief Scale factor to convert 6-bit to 8-bit color values
constexpr std::uint8_t PALETTE_SCALE_FACTOR = 255 / PALETTE_COMPONENT_MAX;  // 4

/// @brief Number of shade tables in a full palette
constexpr std::size_t SHADE_TABLE_COUNT = 32;

/// @brief Number of entries per shade table
constexpr std::size_t SHADE_TABLE_SIZE = PALETTE_SIZE;

/// @brief Total entries in all shade tables
constexpr std::size_t SHADE_TABLE_TOTAL_ENTRIES =
    SHADE_TABLE_COUNT * SHADE_TABLE_SIZE;

/// @brief Size of translucent blend table (64K entries)
constexpr std::size_t TRANSLUCENT_TABLE_SIZE = 65536;

/// @brief Maximum tile width supported (per ART format spec)
constexpr std::uint16_t MAX_TILE_WIDTH = 32767;

/// @brief Maximum tile height supported (per ART format spec)
constexpr std::uint16_t MAX_TILE_HEIGHT = 32767;

/// @brief Maximum tile dimension (used for bounds checking)
constexpr std::uint16_t MAX_TILE_DIMENSION = 32767;

/// @brief Number of bytes per pixel in RGBA format
constexpr std::size_t RGBA_BYTES_PER_PIXEL = 4;

/// @brief Number of color channels in RGBA format
constexpr std::size_t RGBA_CHANNEL_COUNT = 4;

}  // namespace constants

/// @brief Common type aliases used throughout the library
namespace types {

/// @brief Byte type for binary data
using byte = std::byte;

/// @brief 8-bit unsigned integer type
using u8 = std::uint8_t;

/// @brief 8-bit signed integer type
using i8 = std::int8_t;

/// @brief 16-bit unsigned integer type
using u16 = std::uint16_t;

/// @brief 32-bit unsigned integer type
using u32 = std::uint32_t;

/// @brief 64-bit unsigned integer type
using u64 = std::uint64_t;

/// @brief 8-bit signed integer type
using i8 = std::int8_t;

/// @brief 16-bit signed integer type
using i16 = std::int16_t;

/// @brief 32-bit signed integer type
using i32 = std::int32_t;

/// @brief 64-bit signed integer type
using i64 = std::int64_t;

/// @brief Span of constant bytes for read-only binary data
using byte_span = std::span<const std::byte>;

/// @brief Span of mutable bytes for writable binary data
using mutable_byte_span = std::span<std::byte>;

/// @brief Span of constant 8-bit values
using u8_span = std::span<const u8>;

/// @brief Span of mutable 8-bit values
using mutable_u8_span = std::span<u8>;

}  // namespace types

/// @brief Color structures and format handling
namespace color {

/// @brief Color format enumeration for different pixel layouts
enum class Format {
  RGBA,  ///< Red, Green, Blue, Alpha (standard)
  BGRA,  ///< Blue, Green, Red, Alpha (Windows/DirectX)
  ARGB,  ///< Alpha, Red, Green, Blue (some graphics APIs)
  ABGR,  ///< Alpha, Blue, Green, Red (rare)
  RGB,   ///< Red, Green, Blue (no alpha)
  BGR,   ///< Blue, Green, Red (no alpha, common in legacy formats)
};

/// @brief Basic RGBA color structure with normalized 8-bit components
struct Color {
  types::u8 r{0};    ///< Red component (0-255)
  types::u8 g{0};    ///< Green component (0-255)
  types::u8 b{0};    ///< Blue component (0-255)
  types::u8 a{255};  ///< Alpha component (0-255, 255 = opaque)

  /// @brief Default constructor (black, opaque)
  constexpr Color() = default;

  /// @brief Constructor with RGB values (alpha defaults to 255)
  constexpr Color(types::u8 red, types::u8 green, types::u8 blue,
                  types::u8 alpha = 255)
      : r(red), g(green), b(blue), a(alpha) {}

  /// @brief Constructor from 32-bit integer (format-dependent)
  explicit constexpr Color(types::u32 packed, Format format = Format::RGBA) {
    from_packed(packed, format);
  }

  /// @brief Convert to packed 32-bit integer in specified format
  constexpr types::u32 to_packed(Format format = Format::RGBA) const noexcept {
    switch (format) {
      case Format::RGBA:
        return (static_cast<types::u32>(r) << 24) |
               (static_cast<types::u32>(g) << 16) |
               (static_cast<types::u32>(b) << 8) | static_cast<types::u32>(a);
      case Format::BGRA:
        return (static_cast<types::u32>(b) << 24) |
               (static_cast<types::u32>(g) << 16) |
               (static_cast<types::u32>(r) << 8) | static_cast<types::u32>(a);
      case Format::ARGB:
        return (static_cast<types::u32>(a) << 24) |
               (static_cast<types::u32>(r) << 16) |
               (static_cast<types::u32>(g) << 8) | static_cast<types::u32>(b);
      case Format::ABGR:
        return (static_cast<types::u32>(a) << 24) |
               (static_cast<types::u32>(b) << 16) |
               (static_cast<types::u32>(g) << 8) | static_cast<types::u32>(r);
      case Format::RGB:
        return (static_cast<types::u32>(r) << 16) |
               (static_cast<types::u32>(g) << 8) | static_cast<types::u32>(b);
      case Format::BGR:
        return (static_cast<types::u32>(b) << 16) |
               (static_cast<types::u32>(g) << 8) | static_cast<types::u32>(r);
    }
    return 0;  // Should never reach here
  }

  /// @brief Set from packed 32-bit integer in specified format
  constexpr void from_packed(types::u32 packed, Format format) noexcept {
    switch (format) {
      case Format::RGBA:
        r = static_cast<types::u8>((packed >> 24) & 0xFF);
        g = static_cast<types::u8>((packed >> 16) & 0xFF);
        b = static_cast<types::u8>((packed >> 8) & 0xFF);
        a = static_cast<types::u8>(packed & 0xFF);
        break;
      case Format::BGRA:
        b = static_cast<types::u8>((packed >> 24) & 0xFF);
        g = static_cast<types::u8>((packed >> 16) & 0xFF);
        r = static_cast<types::u8>((packed >> 8) & 0xFF);
        a = static_cast<types::u8>(packed & 0xFF);
        break;
      case Format::ARGB:
        a = static_cast<types::u8>((packed >> 24) & 0xFF);
        r = static_cast<types::u8>((packed >> 16) & 0xFF);
        g = static_cast<types::u8>((packed >> 8) & 0xFF);
        b = static_cast<types::u8>(packed & 0xFF);
        break;
      case Format::ABGR:
        a = static_cast<types::u8>((packed >> 24) & 0xFF);
        b = static_cast<types::u8>((packed >> 16) & 0xFF);
        g = static_cast<types::u8>((packed >> 8) & 0xFF);
        r = static_cast<types::u8>(packed & 0xFF);
        break;
      case Format::RGB:
        r = static_cast<types::u8>((packed >> 16) & 0xFF);
        g = static_cast<types::u8>((packed >> 8) & 0xFF);
        b = static_cast<types::u8>(packed & 0xFF);
        a = 255;  // No alpha channel, default to opaque
        break;
      case Format::BGR:
        b = static_cast<types::u8>((packed >> 16) & 0xFF);
        g = static_cast<types::u8>((packed >> 8) & 0xFF);
        r = static_cast<types::u8>(packed & 0xFF);
        a = 255;  // No alpha channel, default to opaque
        break;
    }
  }

  /// @brief Convert from one format to another
  constexpr Color convert_format(Format from_format, Format to_format) const {
    if (from_format == to_format) {
      return *this;
    }
    const types::u32 packed = to_packed(from_format);
    return Color(packed, to_format);
  }

  /// @brief Premultiply alpha (RGB *= A/255)
  constexpr Color premultiplied() const noexcept {
    const types::u16 alpha_factor =
        static_cast<types::u16>(a) + 1;  // +1 for rounding
    return Color(static_cast<types::u8>(
                     (static_cast<types::u16>(r) * alpha_factor) >> 8),
                 static_cast<types::u8>(
                     (static_cast<types::u16>(g) * alpha_factor) >> 8),
                 static_cast<types::u8>(
                     (static_cast<types::u16>(b) * alpha_factor) >> 8),
                 a);
  }

  /// @brief Set alpha to 0 (fully transparent)
  constexpr Color make_transparent() const noexcept {
    return Color(r, g, b, 0);
  }

  /// @brief Check if color is fully transparent
  constexpr bool is_transparent() const noexcept { return a == 0; }

  /// @brief Check if color is fully opaque
  constexpr bool is_opaque() const noexcept { return a == 255; }

  /// @brief Equality comparison
  constexpr bool operator==(const Color& other) const noexcept {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }

  /// @brief Inequality comparison
  constexpr bool operator!=(const Color& other) const noexcept {
    return !(*this == other);
  }
};

/// @brief Common color constants
namespace constants {
constexpr Color BLACK{0, 0, 0, 255};
constexpr Color WHITE{255, 255, 255, 255};
constexpr Color RED{255, 0, 0, 255};
constexpr Color GREEN{0, 255, 0, 255};
constexpr Color BLUE{0, 0, 255, 255};
constexpr Color TRANSPARENT_BLACK{0, 0, 0, 0};
}  // namespace constants

}  // namespace color

/// @brief Image format enumeration for encoding and export
enum class ImageFormat : types::u8 {
  /// @brief Portable Network Graphics format
  png = 0,

  /// @brief Truevision TGA format
  tga = 1,

  /// @brief Windows Bitmap format
  bmp = 2
};

/// @brief Forward declarations for core data structures
struct Palette;
struct TileView;
struct ArtData;
struct Image;
struct ImageView;

}  // namespace art2img
