#include <art2img/core/encode.hpp>

#include <cstddef>
#include <expected>
#include <span>
#include <utility>
#include <vector>

#include <art2img/encode.hpp>
#include <art2img/types.hpp>

namespace art2img::core {

namespace {
::art2img::ImageFormat to_legacy(ImageFormat format) {
  switch (format) {
    case ImageFormat::png:
      return ::art2img::ImageFormat::png;
    case ImageFormat::tga:
      return ::art2img::ImageFormat::tga;
    case ImageFormat::bmp:
      return ::art2img::ImageFormat::bmp;
  }
  return ::art2img::ImageFormat::png;
}

::art2img::ImageView to_legacy_view(const RgbaImageView& view) {
  const auto stride = view.stride == 0 ? view.width * 4u : view.stride;
  return ::art2img::ImageView(
      view.pixels, static_cast<::art2img::types::u16>(view.width),
      static_cast<::art2img::types::u16>(view.height),
      static_cast<std::size_t>(stride));
}

::art2img::EncodeOptions to_options(ImageFormat format,
                                    const EncoderOptions& options) {
  switch (format) {
    case ImageFormat::png: {
      ::art2img::PngOptions png{};
      switch (options.compression) {
        case CompressionPreset::fast:
          png.compression_level = 1;
          png.use_filters = false;
          break;
        case CompressionPreset::smallest:
          png.compression_level = 9;
          png.use_filters = true;
          break;
        case CompressionPreset::balanced:
        default:
          png.compression_level = 6;
          png.use_filters = true;
          break;
      }
      png.convert_to_grayscale = false;
      return png;
    }
    case ImageFormat::tga: {
      ::art2img::TgaOptions tga{};
      tga.use_rle = options.compression != CompressionPreset::fast;
      tga.include_alpha = options.bit_depth != BitDepth::bpp24;
      tga.flip_vertically = false;
      return tga;
    }
    case ImageFormat::bmp: {
      ::art2img::BmpOptions bmp{};
      bmp.include_alpha = options.bit_depth != BitDepth::bpp24;
      bmp.flip_vertically = false;
      return bmp;
    }
  }
  return {};
}

}  // namespace

std::expected<EncodedImage, Error> encode_image(const RgbaImageView& image,
                                                ImageFormat format,
                                                EncoderOptions options) {
  if (!image.valid()) {
    return std::unexpected(
        make_error(errc::conversion_failure, "invalid image view"));
  }

  const auto legacy_view = to_legacy_view(image);
  auto encode_options = to_options(format, options);
  auto encoded = ::art2img::encode_image(legacy_view, to_legacy(format),
                                         std::move(encode_options));
  if (!encoded) {
    return std::unexpected(encoded.error());
  }

  EncodedImage result{};
  result.format = format;
  result.width = image.width;
  result.height = image.height;
  result.bytes = std::move(encoded.value());
  return result;
}

}  // namespace art2img::core
