// test_constants.hpp
//
// Centralized constants for art2img tests.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <string>

namespace art2img::test::constants {

// ============================================================================
// File Signatures
// ============================================================================

inline constexpr const char* GRP_SIGNATURE = "KenSilverman";
inline constexpr size_t GRP_SIGNATURE_LENGTH = 12;

inline constexpr std::array<uint8_t, 8> PNG_MAGIC = {0x89, 0x50, 0x4E, 0x47,
                                                     0x0D, 0x0A, 0x1A, 0x0A};

// ============================================================================
// File Paths
// ============================================================================

inline constexpr const char* SHAREWARE_GRP_PATH = "tests/shareware/DUKE3D.GRP";

// ============================================================================
// Palette Constants
// ============================================================================

inline constexpr size_t PALETTE_BASE_SIZE = 768;
inline constexpr size_t PALETTE_EXTENDED_SIZE = 8960;
inline constexpr size_t SHADE_TABLE_COUNT = 32;
inline constexpr size_t PALETTE_COLOR_COUNT = 256;
inline constexpr size_t BYTES_PER_RGBA = 4;
inline constexpr uint8_t TRANSPARENT_COLOR_INDEX = 255;
inline constexpr uint8_t BACKGROUND_COLOR_INDEX = 0;

// ============================================================================
// Tile Dimensions
// ============================================================================

inline constexpr uint16_t SMALL_TILE_SIZE = 2;
inline constexpr uint16_t LARGE_TILE_DIM = 64;
inline constexpr uint16_t MAX_TILE_DIM = 256;
inline constexpr uint16_t REASONABLE_MAX_WIDTH = 1024;
inline constexpr uint16_t REASONABLE_MAX_HEIGHT = 1024;

// ============================================================================
// Shade Levels
// ============================================================================

inline constexpr uint8_t SHADE_MIN = 0;
inline constexpr uint8_t SHADE_MID = 16;
inline constexpr uint8_t SHADE_MAX = 31;

// ============================================================================
// File Extensions
// ============================================================================

inline constexpr const char* EXT_PNG = ".png";
inline constexpr const char* EXT_TGA = ".tga";
inline constexpr const char* EXT_BMP = ".bmp";
inline constexpr const char* EXT_ART = ".art";
inline constexpr const char* EXT_CON = ".con";
inline constexpr const char* EXT_VOC = ".voc";
inline constexpr const char* EXT_MID = ".mid";
inline constexpr const char* EXT_MAP = ".map";

// ============================================================================
// File Header Sizes
// ============================================================================

inline constexpr size_t ART_HEADER_MIN_SIZE = 16;

// ============================================================================
// Test File Names
// ============================================================================

inline constexpr const char* TILES000_ART = "tiles000.art";
inline constexpr const char* TILES001_ART = "tiles001.art";
inline constexpr const char* TILES002_ART = "tiles002.art";
inline constexpr const char* PALETTE_DAT = "palette.dat";

// ============================================================================
// Expected Counts
// ============================================================================

inline constexpr size_t MIN_GRP_FILE_COUNT = 100;
inline constexpr size_t MIN_ART_FILE_COUNT = 10;

// ============================================================================
// Version
// ============================================================================

inline constexpr const char* EXPECTED_VERSION = "2.0.0";

// ============================================================================
// Helper Arrays
// ============================================================================

inline constexpr std::array<const char*, 3> OUTPUT_FORMATS = {"png", "tga", "bmp"};
inline constexpr std::array<uint8_t, 5> SHADE_TEST_LEVELS = {SHADE_MIN, 8, SHADE_MID, 24,
                                                             SHADE_MAX};

}  // namespace art2img::test::constants
