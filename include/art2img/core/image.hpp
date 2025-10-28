#pragma once

#include <cstdint>
#include <span>
#include <vector>

namespace art2img::core {

struct RgbaImage {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::vector<std::uint8_t> pixels;

  constexpr bool empty() const noexcept { return pixels.empty(); }
};

struct RgbaImageView {
  std::span<const std::uint8_t> pixels;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::uint32_t stride = 0;

  constexpr bool valid() const noexcept
  {
    return width > 0 && height > 0 && stride >= width * 4 &&
           pixels.size() >= static_cast<std::size_t>(stride) * height;
  }
};

inline RgbaImageView make_view(const RgbaImage& image) noexcept
{
  return RgbaImageView{image.pixels, image.width, image.height,
                       image.width * 4u};
}

}  // namespace art2img::core
