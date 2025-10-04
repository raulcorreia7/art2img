#include "image_processor.hpp"

#include <algorithm>
#include <cstdint>
#include <vector>

namespace art2img {
namespace image_processor {

std::vector<uint8_t> convert_to_rgba(const Palette& palette, const ArtFile::Tile& tile,
                                     const uint8_t* pixel_data, size_t pixel_data_size,
                                     const ImageWriter::Options& options) {
  if (pixel_data_size != tile.size()) {
    throw ArtException("Pixel data size mismatch for tile");
  }

  std::vector<uint8_t> rgba_data(static_cast<size_t>(tile.width) * tile.height * 4);

  const std::vector<uint8_t> palette_data = palette.get_bgr_data();

  // ART format: pixels stored by columns (y + x * height)
  // PNG format: pixels stored by rows (x + y * width), top to bottom
  for (int32_t y = 0; y < tile.height; ++y) {
    for (int32_t x = 0; x < tile.width; ++x) {
      // Get pixel index from ART data (column-major)
      uint8_t pixel_index =
          pixel_data[static_cast<size_t>(y) + static_cast<size_t>(x) * tile.height];

      // The palette data is stored in BGR format (RGB -> BGR after scaling)
      // For PNG we need to reverse this: BGR -> RGB
      const uint8_t b8 = palette_data[pixel_index * 3 + 0];  // BGR Blue (was originally red)
      const uint8_t g8 = palette_data[pixel_index * 3 + 1];  // BGR Green (was originally green)
      const uint8_t r8 = palette_data[pixel_index * 3 + 2];  // BGR Red (was originally blue)

      // Calculate position in RGBA buffer (row-major)
      const size_t rgba_index = (static_cast<size_t>(y) * tile.width + static_cast<size_t>(x)) * 4;

      // Step B: Alpha construction (magenta keying)
      uint8_t alpha = 255;
      if (options.enable_alpha && options.fix_transparency && is_build_engine_magenta(r8, g8, b8)) {
        alpha = 0;
      }

      // Store RGBA values
      rgba_data[rgba_index + 0] = r8;     // R
      rgba_data[rgba_index + 1] = g8;     // G
      rgba_data[rgba_index + 2] = b8;     // B
      rgba_data[rgba_index + 3] = alpha;  // A
    }
  }

  // Step C: Apply premultiplication if enabled
  if (options.enable_alpha && options.premultiply_alpha) {
    apply_premultiplication(rgba_data);
  }

  // Step D: Optional matte hygiene
  if (options.enable_alpha && options.matte_hygiene) {
    apply_matte_hygiene(rgba_data, tile.width, tile.height);
    // Re-apply premultiplication after alpha changes
    if (options.premultiply_alpha) {
      apply_premultiplication(rgba_data);
    }
  }

  return rgba_data;
}

void apply_premultiplication(std::vector<uint8_t>& rgba_data) {
  // Step C: Enforce no hidden color + premultiply edges
  for (size_t i = 0; i < rgba_data.size(); i += 4) {
    uint8_t r = rgba_data[i];
    uint8_t g = rgba_data[i + 1];
    uint8_t b = rgba_data[i + 2];
    uint8_t a = rgba_data[i + 3];

    // Rule 1: erase hidden RGB
    if (a == 0) {
      rgba_data[i] = 0;      // R
      rgba_data[i + 1] = 0;  // G
      rgba_data[i + 2] = 0;  // B
    }
    // Rule 2: premultiply edges
    else if (a < 255) {
      // Premultiply with rounding: (r * a + 127) / 255
      rgba_data[i] = static_cast<uint8_t>((r * a + 127) / 255);      // R
      rgba_data[i + 1] = static_cast<uint8_t>((g * a + 127) / 255);  // G
      rgba_data[i + 2] = static_cast<uint8_t>((b * a + 127) / 255);  // B
    }
    // For a=255, RGB values remain unchanged (already premultiplied by 1.0)
  }
}

void apply_matte_hygiene(std::vector<uint8_t>& rgba_data, int width, int height) {
  // Create temporary alpha channel for processing
  std::vector<uint8_t> alpha_channel(static_cast<size_t>(width) * height);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      size_t rgba_index = (static_cast<size_t>(y) * width + x) * 4;
      alpha_channel[static_cast<size_t>(y) * width + x] = rgba_data[rgba_index + 3];
    }
  }

  // Step D: Erode alpha by 1 px with 4-connected neighbors
  std::vector<uint8_t> eroded_alpha = alpha_channel;
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      size_t idx = static_cast<size_t>(y) * width + x;
      if (alpha_channel[idx] > 0) {
        // Check 4-connected neighbors (N, E, S, W)
        uint8_t min_neighbor = 255;
        min_neighbor = std::min(min_neighbor, alpha_channel[idx - width]);  // North
        min_neighbor = std::min(min_neighbor, alpha_channel[idx + 1]);      // East
        min_neighbor = std::min(min_neighbor, alpha_channel[idx + width]);  // South
        min_neighbor = std::min(min_neighbor, alpha_channel[idx - 1]);      // West

        eroded_alpha[idx] = min_neighbor;
      }
    }
  }

  // Step D: Feather alpha with 3×3 box blur
  std::vector<uint8_t> blurred_alpha = eroded_alpha;
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      size_t idx = static_cast<size_t>(y) * width + x;

      // 3x3 box blur
      uint32_t sum = 0;
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          sum += eroded_alpha[idx + static_cast<size_t>(dy) * width + dx];
        }
      }
      blurred_alpha[idx] = static_cast<uint8_t>(sum / 9);
    }
  }

  // Update RGBA data with processed alpha
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      size_t rgba_index = (static_cast<size_t>(y) * width + x) * 4;
      rgba_data[rgba_index + 3] = blurred_alpha[static_cast<size_t>(y) * width + x];
    }
  }
}

}  // namespace image_processor
}  // namespace art2img