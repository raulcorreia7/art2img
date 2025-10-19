#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>
#include <string>

// Include span if available (C++20+)
#if __has_include(<span>)
#include <span>
#endif

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
constexpr std::uint8_t PALETTE_COMPONENT_MAX = (1 << PALETTE_BITS_PER_COMPONENT) - 1; // 63

/// @brief Scale factor to convert 6-bit to 8-bit color values
constexpr std::uint8_t PALETTE_SCALE_FACTOR = 255 / PALETTE_COMPONENT_MAX; // 4

/// @brief Number of shade tables in a full palette
constexpr std::size_t SHADE_TABLE_COUNT = 32;

/// @brief Number of entries per shade table
constexpr std::size_t SHADE_TABLE_SIZE = PALETTE_SIZE;

/// @brief Total entries in all shade tables
constexpr std::size_t SHADE_TABLE_TOTAL_ENTRIES = SHADE_TABLE_COUNT * SHADE_TABLE_SIZE;

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

} // namespace constants

/// @brief Common type aliases used throughout the library
namespace types {

/// @brief Byte type for binary data
using byte = std::byte;

/// @brief 8-bit unsigned integer type
using u8 = std::uint8_t;

/// @brief 16-bit unsigned integer type
using u16 = std::uint16_t;

/// @brief 32-bit unsigned integer type
using u32 = std::uint32_t;

/// @brief 64-bit unsigned integer type
using u64 = std::uint64_t;

/// @brief Span of constant bytes for read-only binary data
using byte_span = std::span<const std::byte>;

/// @brief Span of mutable bytes for writable binary data
using mutable_byte_span = std::span<std::byte>;

/// @brief Span of constant 8-bit values
using u8_span = std::span<const u8>;

/// @brief Span of mutable 8-bit values
using mutable_u8_span = std::span<u8>;

} // namespace types

/// @brief Forward declarations for core data structures
struct Palette;
struct TileView;
struct ArtData;
struct Image;
struct ImageView;

} // namespace art2img