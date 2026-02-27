// ============================================================================
// Test Constants Header
// ============================================================================
// Centralized constants for all art2img tests. Provides file signatures,
// magic numbers, file paths, dimensions, and helper functions used across
// the test suite.
//
// Usage:
//   #include "test_constants.hpp"
//   using namespace art2img::test::constants;
//
// ============================================================================

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace art2img::test::constants {

// ============================================================================
// File Signatures and Magic Numbers
// ============================================================================

/// GRP file format signature (Ken Silverman's Build engine)
inline constexpr const char* GRP_SIGNATURE = "KenSilverman";

/// GRP signature length in bytes
inline constexpr size_t GRP_SIGNATURE_LENGTH = 12;

/// PNG file magic bytes: 0x89 PNG\r\n\x1a\n
inline constexpr std::array<uint8_t, 8> PNG_MAGIC = {0x89, 0x50, 0x4E, 0x47,
                                                     0x0D, 0x0A, 0x1A, 0x0A};

/// BMP file magic bytes: "BM"
inline constexpr std::array<uint8_t, 2> BMP_MAGIC = {0x42, 0x4D};

/// TGA file magic (no signature, but we use type 2 as common)
inline constexpr uint8_t TGA_TYPE_UNCOMPRESSED_RGB = 2;

/// MIDI file magic bytes: "MThd"
inline constexpr std::array<uint8_t, 4> MIDI_MAGIC = {0x4D, 0x54, 0x68, 0x64};

/// VOC file magic bytes: "Creative Voice File"
inline constexpr const char* VOC_SIGNATURE = "Creative Voice File";

// ============================================================================
// File Paths
// ============================================================================

/// Path to shareware GRP for integration tests
inline constexpr const char* SHAREWARE_GRP_PATH = "tests/shareware/DUKE3D.GRP";

/// Path to CLI binary (relative to project root)
inline constexpr const char* CLI_BINARY_PATH = "./build/art2img";

// ============================================================================
// Palette and Color Constants
// ============================================================================

/// Size of base palette data (256 colors * 3 bytes RGB)
inline constexpr size_t PALETTE_BASE_SIZE = 768;

/// Size of extended palette with shade tables (256 colors * 35 bytes)
inline constexpr size_t PALETTE_EXTENDED_SIZE = 8960;

/// Number of shade levels in the shade table
inline constexpr size_t SHADE_TABLE_COUNT = 32;

/// Number of colors in the palette
inline constexpr size_t PALETTE_COLOR_COUNT = 256;

/// Bytes per color (RGB)
inline constexpr size_t BYTES_PER_COLOR = 3;

/// Bytes per color (RGBA)
inline constexpr size_t BYTES_PER_RGBA = 4;

/// Palette index for transparent color (typically color 255)
inline constexpr uint8_t TRANSPARENT_COLOR_INDEX = 255;

/// Palette index for opaque background color (typically color 0)
inline constexpr uint8_t BACKGROUND_COLOR_INDEX = 0;

// ============================================================================
// File Header Sizes
// ============================================================================

/// Minimum size for valid ART file header (version + count + start + end)
inline constexpr size_t ART_HEADER_MIN_SIZE = 16;

/// Minimum size for valid GRP file header (signature + file count)
inline constexpr size_t GRP_HEADER_MIN_SIZE = 16;

/// Size of GRP directory entry (12-byte filename + 4-byte size)
inline constexpr size_t GRP_DIR_ENTRY_SIZE = 16;

/// ART file format version (expected value)
inline constexpr uint32_t ART_VERSION = 1;

// ============================================================================
// Tile Dimensions
// ============================================================================

/// Size for small test tiles (2x2 pixels)
inline constexpr uint16_t SMALL_TILE_SIZE = 2;

/// Standard large tile dimension (64x64 pixels)
inline constexpr uint16_t LARGE_TILE_DIM = 64;

/// Maximum tile dimension supported
inline constexpr uint16_t MAX_TILE_DIM = 256;

/// Reasonable maximum width for sanity checks
inline constexpr uint16_t REASONABLE_MAX_WIDTH = 1024;

/// Reasonable maximum height for sanity checks
inline constexpr uint16_t REASONABLE_MAX_HEIGHT = 1024;

// ============================================================================
// Shade Level Constants
// ============================================================================

/// Minimum shade level (no shading, full brightness)
inline constexpr uint8_t SHADE_MIN = 0;

/// Middle shade level (50% shading)
inline constexpr uint8_t SHADE_MID = 16;

/// Maximum shade level (full shading, darkest)
inline constexpr uint8_t SHADE_MAX = 31;

// ============================================================================
// File Extensions
// ============================================================================

/// ART file extension
inline constexpr const char* EXT_ART = ".art";

/// PNG file extension
inline constexpr const char* EXT_PNG = ".png";

/// BMP file extension
inline constexpr const char* EXT_BMP = ".bmp";

/// TGA file extension
inline constexpr const char* EXT_TGA = ".tga";

/// GRP file extension
inline constexpr const char* EXT_GRP = ".grp";

/// Configuration file extension
inline constexpr const char* EXT_CON = ".con";

/// Palette data file extension
inline constexpr const char* EXT_DAT = ".dat";

/// VOC audio file extension
inline constexpr const char* EXT_VOC = ".voc";

/// MIDI music file extension
inline constexpr const char* EXT_MID = ".mid";

/// Map file extension
inline constexpr const char* EXT_MAP = ".map";

/// Text file extension
inline constexpr const char* EXT_TXT = ".txt";

// ============================================================================
// Test File Names
// ============================================================================

/// Generic test text file name
inline constexpr const char* TEST_FILE_TXT = "test.txt";

/// Test ART file name (single tile)
inline constexpr const char* TEST_FILE_ART = "tiles.art";

/// Tiles000.art - primary ART file in shareware GRP
inline constexpr const char* TILES000_ART = "tiles000.art";

/// Tiles001.art - secondary ART file in shareware GRP
inline constexpr const char* TILES001_ART = "tiles001.art";

/// Tiles002.art - tertiary ART file in shareware GRP
inline constexpr const char* TILES002_ART = "tiles002.art";

/// Palette data file name
inline constexpr const char* PALETTE_DAT = "palette.dat";

// ============================================================================
// Expected File Counts (for shareware GRP validation)
// ============================================================================

/// Minimum expected file count in shareware GRP
inline constexpr size_t MIN_GRP_FILE_COUNT = 100;

/// Minimum expected ART file count in shareware GRP
inline constexpr size_t MIN_ART_FILE_COUNT = 10;

/// Minimum expected MIDI files in shareware GRP
inline constexpr size_t MIN_MID_FILE_COUNT = 6;

/// Minimum expected MAP files in shareware GRP
inline constexpr size_t MIN_MAP_FILE_COUNT = 6;

// ============================================================================
// Version Information
// ============================================================================

/// Expected CLI version string
inline constexpr const char* EXPECTED_VERSION = "2.0.0";

// ============================================================================
// Helper Arrays for Testing
// ============================================================================

/// Standard ART files found in Duke3D GRP
inline constexpr std::array<const char*, 3> ART_FILES = {TILES000_ART, TILES001_ART, TILES002_ART};

/// Configuration files in shareware GRP
inline constexpr std::array<const char*, 3> CON_FILES = {"defs.con", "game.con", "user.con"};

/// Shade levels for testing (min, mid, max, and intermediate values)
inline constexpr std::array<uint8_t, 5> SHADE_TEST_LEVELS = {
    SHADE_MIN,  // Full brightness
    8,          // Light shading
    SHADE_MID,  // Medium shading
    24,         // Heavy shading
    SHADE_MAX   // Full shading
};

/// Supported output formats for conversion tests
inline constexpr std::array<const char*, 3> OUTPUT_FORMATS = {"png", "tga", "bmp"};

/// Supported output format extensions (aligned with OUTPUT_FORMATS)
inline constexpr std::array<const char*, 3> OUTPUT_EXTENSIONS = {EXT_PNG, EXT_TGA, EXT_BMP};

/// All ART file patterns in shareware GRP
inline constexpr std::array<const char*, 10> ALL_ART_PATTERNS = {
    "tiles000.art", "tiles001.art", "tiles002.art", "tiles003.art", "tiles004.art",
    "tiles005.art", "tiles006.art", "tiles007.art", "tiles008.art", "tiles009.art"};

// ============================================================================
// Helper Functions
// ============================================================================

/**
 * @brief Scale a 6-bit color component to 8-bit.
 *
 * Build engine palettes store colors in 6-bit format (0-63).
 * This scales to 8-bit (0-255) for modern RGB display.
 *
 * The formula preserves the full range by replicating the top 2 bits:
 *   scaled = (value << 2) | (value >> 4)
 *
 * @param value 6-bit color value (0-63)
 * @return Scaled 8-bit color value (0-255)
 */
inline constexpr uint8_t scale_6bit_to_8bit(uint8_t value) noexcept {
    // Clamp to 6-bit range
    value &= 0x3F;
    // Scale: shift left 2 bits, OR with top 2 bits shifted right 4
    return static_cast<uint8_t>((value << 2) | (value >> 4));
}

/**
 * @brief Check if a byte array matches PNG magic bytes.
 * @param data Pointer to byte array (must have at least 8 bytes)
 * @return true if data starts with PNG signature
 */
inline bool is_png_signature(const std::byte* data) noexcept {
    if (!data)
        return false;
    for (size_t i = 0; i < PNG_MAGIC.size(); ++i) {
        if (static_cast<uint8_t>(data[i]) != PNG_MAGIC[i]) {
            return false;
        }
    }
    return true;
}

/**
 * @brief Check if a byte array matches BMP magic bytes.
 * @param data Pointer to byte array (must have at least 2 bytes)
 * @return true if data starts with BMP signature
 */
inline bool is_bmp_signature(const std::byte* data) noexcept {
    if (!data)
        return false;
    return static_cast<uint8_t>(data[0]) == BMP_MAGIC[0] &&
           static_cast<uint8_t>(data[1]) == BMP_MAGIC[1];
}

/**
 * @brief Check if data starts with GRP signature.
 * @param data Pointer to byte array
 * @param size Size of data in bytes
 * @return true if data starts with "KenSilverman"
 */
inline bool is_grp_signature(const std::byte* data, size_t size) noexcept {
    if (!data || size < GRP_SIGNATURE_LENGTH)
        return false;
    for (size_t i = 0; i < GRP_SIGNATURE_LENGTH; ++i) {
        if (static_cast<char>(data[i]) != GRP_SIGNATURE[i]) {
            return false;
        }
    }
    return true;
}

}  // namespace art2img::test::constants
