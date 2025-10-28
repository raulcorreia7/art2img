#pragma once

#include <cstdint>
#include <span>

namespace art2img::core {

/**
 * @brief Check if RGB values match Build Engine magenta (252, 0, 252) with
 * tolerance
 *
 * The Build Engine uses a specific shade of magenta for transparency.
 * This function detects magenta pixels with tolerance for slight variations.
 *
 * @param r Red channel value (0-255)
 * @param g Green channel value (0-255)
 * @param b Blue channel value (0-255)
 * @return true if the color matches Build Engine magenta within tolerance
 */
constexpr inline bool is_build_engine_magenta(std::uint8_t r,
                                              std::uint8_t g,
                                              std::uint8_t b) noexcept
{
  return r >= 250u && b >= 250u && g <= 5u;
}

/**
 * @brief Check if RGBA data contains any Build Engine magenta pixels
 *
 * This function scans RGBA pixel data and returns true if any pixels
 * contain Build Engine magenta colors. Useful for testing that transparency
 * fixes have been applied correctly.
 *
 * @param rgba_data RGBA pixel data (4 bytes per pixel: R,G,B,A)
 * @return true if any magenta pixels are found
 */
inline bool contains_build_engine_magenta(
    std::span<const std::uint8_t> rgba_data) noexcept
{
  for (std::size_t i = 0; i + 3 < rgba_data.size(); i += 4) {
    if (rgba_data[i + 3] > 0) {  // Only check non-transparent pixels
      if (is_build_engine_magenta(rgba_data[i], rgba_data[i + 1],
                                  rgba_data[i + 2])) {
        return true;
      }
    }
  }
  return false;
}

/**
 * @brief Count Build Engine magenta pixels in RGBA data
 *
 * @param rgba_data RGBA pixel data (4 bytes per pixel: R,G,B,A)
 * @return Number of magenta pixels found
 */
inline std::size_t count_build_engine_magenta(
    std::span<const std::uint8_t> rgba_data) noexcept
{
  std::size_t count = 0;
  for (std::size_t i = 0; i + 3 < rgba_data.size(); i += 4) {
    if (rgba_data[i + 3] > 0) {  // Only count non-transparent pixels
      if (is_build_engine_magenta(rgba_data[i], rgba_data[i + 1],
                                  rgba_data[i + 2])) {
        ++count;
      }
    }
  }
  return count;
}

}  // namespace art2img::core