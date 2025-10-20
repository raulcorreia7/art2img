#pragma once

#include <art2img/color_helpers.hpp>
#include <art2img/types.hpp>
#include <cstring>
#include <vector>

namespace art2img::detail {

/// @brief Write RGBA value to destination buffer (RGBA format)
/// @param dest Destination buffer
/// @param offset Offset in buffer to write to
/// @param color Color to write
inline void write_rgba(types::mutable_u8_span dest, std::size_t offset,
                       const color::Color& color) noexcept {
  if (offset + constants::RGBA_BYTES_PER_PIXEL <= dest.size()) {
    color::write_rgba(dest.data() + offset, color);
  }
}

/// @brief Flip contiguous image data vertically
/// @param data Source image data
/// @param width Image width in pixels
/// @param height Image height in pixels
/// @param channels Number of channels per pixel
/// @return Vector containing flipped image data
inline std::vector<types::u8> flip_image_vertically(const types::u8* data,
                                                    types::u16 width,
                                                    types::u16 height,
                                                    int channels) {
  const std::size_t row_bytes =
      static_cast<std::size_t>(width) * static_cast<std::size_t>(channels);
  std::vector<types::u8> flipped(static_cast<std::size_t>(height) * row_bytes);
  for (types::u16 y = 0; y < height; ++y) {
    const std::size_t dst_row = static_cast<std::size_t>(y) * row_bytes;
    const std::size_t src_row =
        static_cast<std::size_t>(height - 1 - y) * row_bytes;
    std::memcpy(flipped.data() + dst_row, data + src_row, row_bytes);
  }
  return flipped;
}

/// @brief For pixels that are fully transparent, set them to neutral color to
/// prevent halo effects
/// @param rgba_data RGBA image data to process
/// @param width Image width in pixels
/// @param height Image height in pixels
inline void clean_transparent_pixels(std::vector<types::u8>& rgba_data,
                                     types::u16 width, types::u16 height) {
  for (types::u16 y = 0; y < height; ++y) {
    for (types::u16 x = 0; x < width; ++x) {
      const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
      if (idx + 3 < rgba_data.size() &&
          rgba_data[idx + 3] == 0) {  // Alpha channel is 0 (fully transparent)
        rgba_data[idx] = 128;         // R = 128 (neutral gray)
        rgba_data[idx + 1] = 128;     // G = 128 (neutral gray)
        rgba_data[idx + 2] = 128;     // B = 128 (neutral gray)
        // Alpha remains 0
      }
    }
  }
}

/// @brief Apply matte hygiene (erosion + blur) to remove halo effects from
/// alpha channel
/// @param rgba_data RGBA image data to process
/// @param width Image width in pixels
/// @param height Image height in pixels
inline void apply_matte_hygiene(std::vector<types::u8>& rgba_data,
                                types::u16 width, types::u16 height) {
  // Extract alpha channel
  std::vector<types::u8> alpha(static_cast<std::size_t>(width) * height);
  for (types::u16 y = 0; y < height; ++y) {
    for (types::u16 x = 0; x < width; ++x) {
      const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
      if (idx + 3 < rgba_data.size()) {
        alpha[static_cast<std::size_t>(y) * width + x] = rgba_data[idx + 3];
      }
    }
  }

  // Simple erosion: for each pixel, if any neighbor has 0 alpha, set this pixel
  // to 0 alpha
  std::vector<types::u8> eroded_alpha = alpha;
  for (types::u16 y = 1; y < height - 1; ++y) {
    for (types::u16 x = 1; x < width - 1; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * width + x;
      if (alpha[idx] > 0) {  // If current pixel is not fully transparent
        // Check 4-connected neighbors
        if (alpha[idx - 1] == 0 ||      // Left
            alpha[idx + 1] == 0 ||      // Right
            alpha[idx - width] == 0 ||  // Top
            alpha[idx + width] == 0) {  // Bottom
          eroded_alpha[idx] = 0;        // Make this pixel fully transparent
        }
      }
    }
  }

  // Simple blur: average with neighbors
  std::vector<types::u8> blurred_alpha(static_cast<std::size_t>(width) * height,
                                       0);
  for (types::u16 y = 1; y < height - 1; ++y) {
    for (types::u16 x = 1; x < width - 1; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * width + x;
      // Average with 4-connected neighbors
      const int sum = static_cast<int>(eroded_alpha[idx]) +
                      static_cast<int>(eroded_alpha[idx - 1]) +      // Left
                      static_cast<int>(eroded_alpha[idx + 1]) +      // Right
                      static_cast<int>(eroded_alpha[idx - width]) +  // Top
                      static_cast<int>(eroded_alpha[idx + width]);   // Bottom
      blurred_alpha[idx] = static_cast<types::u8>(sum / 5);
    }
  }

  // Apply processed alpha back to image
  for (types::u16 y = 0; y < height; ++y) {
    for (types::u16 x = 0; x < width; ++x) {
      const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
      if (idx + 3 < rgba_data.size()) {
        rgba_data[idx + 3] =
            blurred_alpha[static_cast<std::size_t>(y) * width + x];
      }
    }
  }
}

}  // namespace art2img::detail