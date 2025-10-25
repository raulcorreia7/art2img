#include <art2img/core/convert.hpp>

#include <algorithm>
#include <cstdint>
#include <expected>
#include <optional>
#include <span>
#include <utility>
#include <vector>

#include <art2img/convert.hpp>
#include <art2img/types.hpp>

#include <art2img/core/art.hpp>
#include <art2img/core/palette.hpp>

namespace art2img::core {

namespace {
::art2img::ConversionOptions to_internal(const ConversionOptions& convert,
                                         const PostprocessOptions& post) {
  ::art2img::ConversionOptions options{};
  options.apply_lookup = convert.apply_lookup;
  options.shade_index = convert.shade_index.value_or(0);
  options.fix_transparency = post.apply_transparency_fix;
  options.premultiply_alpha = post.premultiply_alpha;
  options.matte_hygiene = post.sanitize_matte;
  return options;
}

::art2img::TileView make_tile(const TileView& tile) {
  ::art2img::TileView legacy{};
  legacy.width = static_cast<::art2img::types::u16>(tile.width);
  legacy.height = static_cast<::art2img::types::u16>(tile.height);
  const auto* from_archive = detail_access(tile);
  if (from_archive) {
    legacy = *from_archive;
  } else {
    legacy.pixels = std::span(reinterpret_cast<const ::art2img::types::u8*>(
                                    tile.indices.data()),
                              tile.indices.size());
  }
  return legacy;
}

}  // namespace

std::expected<RgbaImage, Error> palette_to_rgba(
    const TileView& tile, PaletteView palette_view,
    ConversionOptions options) {
  const auto palette_impl = detail_access(palette_view);
  if (!palette_impl) {
    return std::unexpected(
        make_error(errc::invalid_palette, "palette view has no backing data"));
  }

  const std::size_t required_pixels = static_cast<std::size_t>(tile.width) *
                                      static_cast<std::size_t>(tile.height);
  if (tile.indices.size() < required_pixels) {
    return std::unexpected(make_error(errc::conversion_failure,
                                      "tile view has insufficient pixel data"));
  }

  const PostprocessOptions postprocess_defaults{
      .apply_transparency_fix = false,
      .premultiply_alpha = false,
      .sanitize_matte = false};
  auto internal = to_internal(options, postprocess_defaults);
  const auto legacy_tile = make_tile(tile);

  auto result = ::art2img::to_rgba(legacy_tile, *palette_impl, internal);
  if (!result) {
    return std::unexpected(result.error());
  }

  RgbaImage image{};
  image.width = result->width;
  image.height = result->height;
  image.pixels = std::move(result->data);
  return image;
}

namespace {
void clean_transparent_pixels(std::vector<std::uint8_t>& pixels,
                              std::uint32_t width,
                              std::uint32_t height) {
  if (width == 0 || height == 0) {
    return;
  }
  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t idx =
          static_cast<std::size_t>(y) * width * 4 + static_cast<std::size_t>(x) * 4;
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

void premultiply_alpha(std::vector<std::uint8_t>& pixels) {
  for (std::size_t i = 0; i + 3 < pixels.size(); i += 4) {
    const auto alpha = pixels[i + 3];
    if (alpha == 0) {
      pixels[i + 0] = 0;
      pixels[i + 1] = 0;
      pixels[i + 2] = 0;
    } else if (alpha < 255) {
      pixels[i + 0] = static_cast<std::uint8_t>((pixels[i + 0] * alpha + 127) / 255);
      pixels[i + 1] = static_cast<std::uint8_t>((pixels[i + 1] * alpha + 127) / 255);
      pixels[i + 2] = static_cast<std::uint8_t>((pixels[i + 2] * alpha + 127) / 255);
    }
  }
}

void apply_matte(std::vector<std::uint8_t>& pixels, std::uint32_t width,
                 std::uint32_t height) {
  if (width < 3 || height < 3) {
    return;
  }
  std::vector<std::uint8_t> alpha(width * height);
  for (std::uint32_t y = 0; y < height; ++y) {
    for (std::uint32_t x = 0; x < width; ++x) {
      const std::size_t idx =
          static_cast<std::size_t>(y) * width * 4 + static_cast<std::size_t>(x) * 4;
      alpha[static_cast<std::size_t>(y) * width + x] =
          (idx + 3 < pixels.size()) ? pixels[idx + 3] : 0;
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
      const std::size_t idx =
          static_cast<std::size_t>(y) * width * 4 + static_cast<std::size_t>(x) * 4;
      if (idx + 3 < pixels.size()) {
        pixels[idx + 3] = blurred[static_cast<std::size_t>(y) * width + x];
      }
    }
  }
}

}  // namespace

void postprocess_rgba(RgbaImage& image, PostprocessOptions options) {
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
