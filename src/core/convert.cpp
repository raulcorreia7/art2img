#include <art2img/core/convert.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <vector>

#include <art2img/core/art.hpp>
#include <art2img/core/palette.hpp>

namespace art2img::core {
namespace {
constexpr std::size_t kChannels = 4;

std::uint8_t expand_component(std::uint8_t value) noexcept
{
  return static_cast<std::uint8_t>((value << 2) | (value >> 4));
}

std::uint8_t apply_lookup(std::uint8_t index,
                          const TileView& tile,
                          const ConversionOptions& options) noexcept
{
  if (!options.apply_lookup || tile.lookup.empty() ||
      index >= tile.lookup.size()) {
    return index;
  }
  return std::to_integer<std::uint8_t>(tile.lookup[index]);
}

std::uint8_t apply_shade(std::uint8_t index,
                         PaletteView palette,
                         const ConversionOptions& options) noexcept
{
  if (!options.shade_index || !palette.has_shades() ||
      palette.shade_tables.empty()) {
    return index;
  }

  const auto shade = std::min<std::uint8_t>(
      *options.shade_index,
      static_cast<std::uint8_t>(palette.shade_table_count - 1));
  const std::size_t offset =
      static_cast<std::size_t>(shade) * shade_table_size +
      static_cast<std::size_t>(index);
  if (offset >= palette.shade_tables.size()) {
    return index;
  }
  return palette.shade_tables[offset];
}

struct RgbaPixel {
  std::uint8_t r = 0;
  std::uint8_t g = 0;
  std::uint8_t b = 0;
  std::uint8_t a = 255;
};

RgbaPixel sample_color(std::uint8_t index,
                       PaletteView palette,
                       const ConversionOptions& options,
                       const TileView& tile) noexcept
{
  auto mapped = apply_lookup(index, tile, options);
  auto shaded = apply_shade(mapped, palette, options);

  const std::size_t base = static_cast<std::size_t>(shaded) * 3;
  RgbaPixel pixel{};
  if (base + 2 < palette.rgb.size()) {
    pixel.r = expand_component(palette.rgb[base + 0]);
    pixel.g = expand_component(palette.rgb[base + 1]);
    pixel.b = expand_component(palette.rgb[base + 2]);
  }

  // ART format uses palette index 255 for transparency, not index 0
  // Only apply transparency detection if fix_transparency is enabled
  if (options.fix_transparency &&
      (shaded == 255 || is_build_engine_magenta(pixel.r, pixel.g, pixel.b))) {
    pixel.a = 0;
    pixel.r = 0;
    pixel.g = 0;
    pixel.b = 0;
  }

  return pixel;
}

void clean_transparent_pixels(std::vector<std::uint8_t>& pixels,
                              std::uint32_t width,
                              std::uint32_t height)
{
  if (width == 0 || height == 0) {
    return;
  }

  const std::size_t row_stride = static_cast<std::size_t>(width) * kChannels;
  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * row_stride +
                              static_cast<std::size_t>(x) * kChannels;
      if (idx + 3 >= pixels.size()) {
        continue;
      }
      if (pixels[idx + 3] == 0) {
        pixels[idx + 0] = 0;
        pixels[idx + 1] = 0;
        pixels[idx + 2] = 0;
      }
    }
  }
}

void premultiply_alpha(std::vector<std::uint8_t>& pixels)
{
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

void apply_matte(std::vector<std::uint8_t>& pixels,
                 std::uint32_t width,
                 std::uint32_t height)
{
  if (width < 3 || height < 3) {
    return;
  }

  const std::size_t count = static_cast<std::size_t>(width) * height;
  std::vector<std::uint8_t> alpha(count, 0);
  const std::size_t row_stride = static_cast<std::size_t>(width) * kChannels;

  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t dst = static_cast<std::size_t>(y) * width + x;
      const std::size_t src = static_cast<std::size_t>(y) * row_stride +
                              static_cast<std::size_t>(x) * kChannels + 3;
      if (src < pixels.size()) {
        alpha[dst] = pixels[src];
      }
    }
  }

  std::vector<std::uint8_t> eroded = alpha;
  for (std::uint32_t y = 1; y + 1 < height; ++y) {
    for (std::uint32_t x = 1; x + 1 < width; ++x) {
      const std::size_t idx = static_cast<std::size_t>(y) * width + x;
      if (alpha[idx] == 0) {
        continue;
      }
      std::uint8_t min_value = 255;
      min_value = std::min(min_value, alpha[idx - width]);
      min_value = std::min(min_value, alpha[idx + width]);
      min_value = std::min(min_value, alpha[idx - 1]);
      min_value = std::min(min_value, alpha[idx + 1]);
      eroded[idx] = min_value;
    }
  }

  std::vector<std::uint8_t> blurred = eroded;
  for (std::uint32_t y = 1; y + 1 < height; ++y) {
    for (std::uint32_t x = 1; x + 1 < width; ++x) {
      std::uint32_t sum = 0;
      for (int dy = -1; dy <= 1; ++dy) {
        for (int dx = -1; dx <= 1; ++dx) {
          sum += eroded[static_cast<std::size_t>(y + dy) * width + (x + dx)];
        }
      }
      blurred[static_cast<std::size_t>(y) * width + x] =
          static_cast<std::uint8_t>(sum / 9);
    }
  }

  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t dst = static_cast<std::size_t>(y) * row_stride +
                              static_cast<std::size_t>(x) * kChannels + 3;
      if (dst < pixels.size()) {
        pixels[dst] = blurred[static_cast<std::size_t>(y) * width +
                              static_cast<std::size_t>(x)];
      }
    }
  }
}

}  // namespace

std::expected<RgbaImage, Error> palette_to_rgba(const TileView& tile,
                                                PaletteView palette_view,
                                                ConversionOptions options)
{
  if (!tile.valid()) {
    return std::unexpected(
        make_error(errc::conversion_failure, "invalid tile view"));
  }
  if (palette_view.rgb.size() < palette_component_count) {
    return std::unexpected(
        make_error(errc::invalid_palette, "palette view missing color data"));
  }

  const auto required = static_cast<std::size_t>(tile.width) * tile.height;
  if (tile.indices.size() < required) {
    return std::unexpected(make_error(errc::conversion_failure,
                                      "tile does not contain enough indices"));
  }

  RgbaImage image{};
  image.width = tile.width;
  image.height = tile.height;
  image.pixels.resize(required * kChannels);

  const std::size_t row_stride =
      static_cast<std::size_t>(image.width) * kChannels;
  for (std::uint32_t y = 0; y < tile.height; ++y) {
    for (std::uint32_t x = 0; x < tile.width; ++x) {
      const std::size_t src_index =
          static_cast<std::size_t>(x) * tile.height + y;
      const auto palette_index =
          std::to_integer<std::uint8_t>(tile.indices[src_index]);
      const auto color =
          sample_color(palette_index, palette_view, options, tile);

      const std::size_t dst = static_cast<std::size_t>(y) * row_stride +
                              static_cast<std::size_t>(x) * kChannels;
      image.pixels[dst + 0] = color.r;
      image.pixels[dst + 1] = color.g;
      image.pixels[dst + 2] = color.b;
      image.pixels[dst + 3] = color.a;
    }
  }

  // Apply transparency processing pipeline
  if (options.fix_transparency) {
    clean_transparent_pixels(image.pixels, image.width, image.height);
  }

  if (options.matte_hygiene) {
    apply_matte(image.pixels, image.width, image.height);
  }

  if (options.premultiply_alpha) {
    premultiply_alpha(image.pixels);
  }

  return image;
}

void postprocess_rgba(RgbaImage& image, PostprocessOptions options)
{
  if (image.width == 0 || image.height == 0 || image.pixels.empty()) {
    return;
  }

  if (options.apply_transparency_fix) {
    clean_transparent_pixels(image.pixels, image.width, image.height);
  }

  if (options.sanitize_matte) {
    apply_matte(image.pixels, image.width, image.height);
  }

  if (options.premultiply_alpha) {
    premultiply_alpha(image.pixels);
  }
}

}  // namespace art2img::core
