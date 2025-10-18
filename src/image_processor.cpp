#include "art2img/image_processor.hpp"

#include <algorithm>
#include <cstdint>
#include <span>
#include <vector>

#include "art2img/exceptions.hpp"
#include "art2img/image_writer.hpp"

namespace art2img {
namespace {

constexpr bool is_build_engine_magenta(uint8_t r, uint8_t g, uint8_t b) {
  return (r >= 250u) && (b >= 250u) && (g <= 5u);
}

// For pixels that are fully transparent, set them to a neutral color to prevent color bleeding
void clean_transparent_pixels(std::vector<uint8_t>& rgba_data, int width, int height) {
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
      if (rgba_data[idx + 3] == 0) {  // Fully transparent pixel
        // Set RGB to black to prevent magenta bleeding
        rgba_data[idx + 0] = 0;  // R
        rgba_data[idx + 1] = 0;  // G
        rgba_data[idx + 2] = 0;  // B
      }
    }
  }
}

void apply_premultiplication(std::vector<uint8_t>& rgba_data) {
  for (std::size_t i = 0; i < rgba_data.size(); i += 4) {
    const uint8_t alpha = rgba_data[i + 3];
    if (alpha == 0) {
      rgba_data[i + 0] = 0;
      rgba_data[i + 1] = 0;
      rgba_data[i + 2] = 0;
    } else if (alpha < 255) {
      rgba_data[i + 0] = static_cast<uint8_t>((rgba_data[i + 0] * alpha + 127) / 255);
      rgba_data[i + 1] = static_cast<uint8_t>((rgba_data[i + 1] * alpha + 127) / 255);
      rgba_data[i + 2] = static_cast<uint8_t>((rgba_data[i + 2] * alpha + 127) / 255);
    }
  }
}

void apply_matte_hygiene(std::vector<uint8_t>& rgba_data, int width, int height) {
  std::vector<uint8_t> alpha(static_cast<std::size_t>(width) * height);
  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
      alpha[static_cast<std::size_t>(y) * width + x] = rgba_data[idx + 3];
    }
  }

  std::vector<uint8_t> eroded = alpha;
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * width + x;
      if (alpha[idx] > 0) {
        uint8_t min_neighbor = 255;
        min_neighbor = std::min(min_neighbor, alpha[idx - width]);
        min_neighbor = std::min(min_neighbor, alpha[idx + width]);
        min_neighbor = std::min(min_neighbor, alpha[idx - 1]);
        min_neighbor = std::min(min_neighbor, alpha[idx + 1]);
        eroded[idx] = min_neighbor;
      }
    }
  }

  std::vector<uint8_t> blurred = eroded;
  for (int y = 1; y < height - 1; ++y) {
    for (int x = 1; x < width - 1; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * width + x;
      uint32_t sum = 0;
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          sum += eroded[idx + static_cast<std::size_t>(dy) * width + dx];
        }
      }
      blurred[idx] = static_cast<uint8_t>(sum / 9);
    }
  }

  for (int y = 0; y < height; ++y) {
    for (int x = 0; x < width; ++x) {
      const std::size_t idx = (static_cast<std::size_t>(y) * width + x) * 4;
      rgba_data[idx + 3] = blurred[static_cast<std::size_t>(y) * width + x];
    }
  }
}

std::vector<uint8_t> convert_pixels_to_rgba(const Palette& palette, uint16_t width, uint16_t height,
                                            const uint8_t* pixels, std::size_t pixel_count,
                                            const TileConversionOptions& options) {
  const std::size_t expected = static_cast<std::size_t>(width) * height;
  if (pixel_count != expected) {
    throw ArtException("Pixel data size mismatch for tile dimensions");
  }

  std::vector<uint8_t> rgba(expected * 4);
  const std::vector<uint8_t> palette_data = palette.get_bgr_data();

  for (int32_t y = 0; y < height; ++y) {
    for (int32_t x = 0; x < width; ++x) {
      const uint8_t palette_index =
          pixels[static_cast<std::size_t>(y) + static_cast<std::size_t>(x) * height];
      const uint8_t b = palette_data[static_cast<std::size_t>(palette_index) * 3 + 0];
      const uint8_t g = palette_data[static_cast<std::size_t>(palette_index) * 3 + 1];
      const uint8_t r = palette_data[static_cast<std::size_t>(palette_index) * 3 + 2];
      const std::size_t rgba_index =
          (static_cast<std::size_t>(y) * width + static_cast<std::size_t>(x)) * 4;

      uint8_t alpha = 255;
      uint8_t out_r = r;
      uint8_t out_g = g;
      uint8_t out_b = b;

      if (options.enable_alpha && options.fix_transparency && is_build_engine_magenta(r, g, b)) {
        alpha = 0;
        // Set RGB to black for fully transparent pixels to prevent color bleeding
        out_r = 0;
        out_g = 0;
        out_b = 0;
      }

      rgba[rgba_index + 0] = out_r;
      rgba[rgba_index + 1] = out_g;
      rgba[rgba_index + 2] = out_b;
      rgba[rgba_index + 3] = alpha;
    }
  }

  if (options.enable_alpha && options.premultiply_alpha) {
    apply_premultiplication(rgba);
  }

  if (options.enable_alpha && options.apply_matte_hygiene) {
    apply_matte_hygiene(rgba, width, height);
    if (options.premultiply_alpha) {
      apply_premultiplication(rgba);
    }
  }

  return rgba;
}

}  // namespace

std::vector<uint8_t> convert_tile_to_rgba(const Palette& palette, const TileView& tile,
                                          const TileConversionOptions& options) {
  if (tile.width == 0 || tile.height == 0) {
    throw ArtException("Tile dimensions must be positive");
  }

  const std::size_t expected = tile.pixel_count();
  if (tile.pixels.size() != expected) {
    throw ArtException("Indexed tile payload does not match expected dimensions");
  }

  const uint8_t* source_pixels = tile.pixels.data();
  std::vector<uint8_t> remapped_pixels;

  if (tile.has_lookup()) {
    remapped_pixels.assign(tile.pixels.begin(), tile.pixels.end());
    for (auto& value : remapped_pixels) {
      const std::size_t index = static_cast<std::size_t>(value);
      if (index >= tile.lookup.size()) {
        throw ArtException("Lookup table too small for palette index");
      }
      value = tile.lookup[index];
    }
    source_pixels = remapped_pixels.data();
  }

  return convert_pixels_to_rgba(palette, tile.width, tile.height, source_pixels, tile.pixels.size(),
                                options);
}

}  // namespace art2img

// Legacy compatibility namespace
namespace image_processor {

std::vector<uint8_t> convert_to_rgba(const art2img::Palette& palette,
                                     const art2img::ArtFile::Tile& tile, const uint8_t* pixel_data,
                                     size_t pixel_data_size,
                                     const art2img::ImageWriter::Options& options) {
  // Convert the old API to the new TileView API
  if (pixel_data_size != tile.size()) {
    throw art2img::ArtException("Pixel data size mismatch for tile");
  }

  // Create TileView from legacy data
  art2img::TileView tile_view;
  tile_view.width = tile.width;
  tile_view.height = tile.height;
  tile_view.pixels = std::span<const uint8_t>(pixel_data, pixel_data_size);
  // No lookup table for legacy API - use empty span

  // Convert ImageWriter::Options to TileConversionOptions
  art2img::TileConversionOptions conv_options;
  conv_options.enable_alpha = options.enable_alpha;
  conv_options.fix_transparency = options.fix_transparency;
  conv_options.premultiply_alpha = options.premultiply_alpha;
  conv_options.apply_matte_hygiene = options.matte_hygiene;

  // Call the new API
  return art2img::convert_tile_to_rgba(palette, tile_view, conv_options);
}

}  // namespace image_processor
