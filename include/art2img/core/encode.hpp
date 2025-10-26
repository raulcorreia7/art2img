#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <string_view>
#include <vector>

#include "convert.hpp"
#include "error.hpp"
#include "image.hpp"

namespace art2img::core {

enum class ImageFormat : std::uint8_t { png, tga, bmp };

enum class CompressionPreset : std::uint8_t { balanced, fast, smallest };

enum class BitDepth : std::uint8_t { auto_detect, bpp24, bpp32 };

struct EncoderOptions {
  CompressionPreset compression = CompressionPreset::balanced;
  BitDepth bit_depth = BitDepth::auto_detect;
};

struct EncodedImage {
  ImageFormat format = ImageFormat::png;
  std::uint32_t width = 0;
  std::uint32_t height = 0;
  std::vector<std::byte> bytes;
};

std::expected<EncodedImage, Error> encode_image(const RgbaImageView& image,
                                                ImageFormat format,
                                                EncoderOptions options = {});

constexpr std::string_view file_extension(ImageFormat format) noexcept {
  switch (format) {
    case ImageFormat::png:
      return "png";
    case ImageFormat::tga:
      return "tga";
    case ImageFormat::bmp:
      return "bmp";
  }
  return "bin";
}

}  // namespace art2img::core
