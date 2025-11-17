#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <vector>

namespace art2img::core {

// ============================================================================
// COLOR CONSTANTS AND UTILITIES
// ============================================================================

inline constexpr std::uint8_t kBuildEngineMagentaR = 252;
inline constexpr std::uint8_t kBuildEngineMagentaG = 0;
inline constexpr std::uint8_t kBuildEngineMagentaB = 252;
inline constexpr std::uint8_t kTransparencyIndex = 255;

inline constexpr std::uint8_t kComponentExpandShift = 2;
inline constexpr std::uint8_t kComponentExpandMask = 0x0F;

// ============================================================================
// CORE COLOR MANIPULATION
// ============================================================================

/**
 * @brief Expand 6-bit color component to 8-bit
 * @param value 6-bit color component (0-63)
 * @return 8-bit color component (0-255)
 */
constexpr inline std::uint8_t expand_color_component(
    std::uint8_t value) noexcept
{
  return static_cast<std::uint8_t>((value << 2) | (value >> 4));
}

/**
 * @brief Check if RGB values match Build Engine magenta with tolerance
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
 * @brief Check if RGBA pixel represents transparency
 * @param index Palette index
 * @param r Red channel value
 * @param g Green channel value
 * @param b Blue channel value
 * @return true if pixel should be transparent
 */
constexpr inline bool is_transparent_pixel(std::uint8_t index,
                                           std::uint8_t r,
                                           std::uint8_t g,
                                           std::uint8_t b) noexcept
{
  return index == kTransparencyIndex || is_build_engine_magenta(r, g, b);
}

// ============================================================================
// PIXEL PROCESSING UTILITIES
// ============================================================================

/**
 * @brief RGBA pixel structure for color processing
 */
struct RgbaPixel {
  std::uint8_t r = 0;
  std::uint8_t g = 0;
  std::uint8_t b = 0;
  std::uint8_t a = 255;

  constexpr bool operator==(const RgbaPixel& other) const noexcept
  {
    return r == other.r && g == other.g && b == other.b && a == other.a;
  }

  constexpr bool operator!=(const RgbaPixel& other) const noexcept
  {
    return !(*this == other);
  }
};

/**
 * @brief Create RGBA pixel from palette index and color data
 * @param index Palette index
 * @param palette_rgb Palette RGB data pointer
 * @param palette_size Size of palette data
 * @param offset Byte offset into palette data
 * @return RGBA pixel with expanded color components
 */
constexpr inline RgbaPixel make_pixel_from_palette(
    std::uint8_t index,
    const std::uint8_t* palette_rgb,
    std::size_t palette_size,
    std::size_t offset) noexcept
{
  RgbaPixel pixel{};
  if (offset + 2 < palette_size) {
    pixel.r = expand_color_component(palette_rgb[offset + 0]);
    pixel.g = expand_color_component(palette_rgb[offset + 1]);
    pixel.b = expand_color_component(palette_rgb[offset + 2]);
  }
  return pixel;
}

/**
 * @brief Apply transparency to RGBA pixel
 * @param pixel Pixel to modify
 * @param index Original palette index
 */
constexpr inline void apply_transparency(RgbaPixel& pixel,
                                         std::uint8_t index) noexcept
{
  if (is_transparent_pixel(index, pixel.r, pixel.g, pixel.b)) {
    pixel.a = 0;
    pixel.r = 0;
    pixel.g = 0;
    pixel.b = 0;
  }
}

// ============================================================================
// BUFFER PROCESSING
// ============================================================================

/**
 * @brief Check if RGBA data contains any Build Engine magenta pixels
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

// ============================================================================
// ALPHA PROCESSING
// ============================================================================

/**
 * @brief Premultiply RGB channels by alpha channel
 * @param pixels RGBA pixel buffer
 */
inline void premultiply_alpha(std::vector<std::uint8_t>& pixels) noexcept
{
  constexpr std::size_t kChannels = 4;
  for (std::size_t i = 0; i + 3 < pixels.size(); i += kChannels) {
    const auto alpha = pixels[i + 3];
    if (alpha == 0) {
      pixels[i + 0] = 0;
      pixels[i + 1] = 0;
      pixels[i + 2] = 0;
    }
    else if (alpha < 255) {
      pixels[i + 0] =
          static_cast<std::uint8_t>((pixels[i + 0] * alpha + 127) / 255);
      pixels[i + 1] =
          static_cast<std::uint8_t>((pixels[i + 1] * alpha + 127) / 255);
      pixels[i + 2] =
          static_cast<std::uint8_t>((pixels[i + 2] * alpha + 127) / 255);
    }
  }
}

/**
 * @brief Clean transparent pixels by setting RGB to black
 * @param pixels RGBA pixel buffer
 * @param width Image width
 * @param height Image height
 */
inline void clean_transparent_pixels(std::vector<std::uint8_t>& pixels,
                                     std::uint32_t width,
                                     std::uint32_t height) noexcept
{
  if (width == 0 || height == 0) {
    return;
  }

  constexpr std::size_t kChannels = 4;
  const std::size_t row_stride = static_cast<std::size_t>(width) * kChannels;

  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * row_stride +
                              static_cast<std::size_t>(x) * kChannels;
      if (idx + 3 < pixels.size() && pixels[idx + 3] == 0) {
        pixels[idx + 0] = 0;
        pixels[idx + 1] = 0;
        pixels[idx + 2] = 0;
      }
    }
  }
}

}  // namespace art2img::core