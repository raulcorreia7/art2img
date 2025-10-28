#include <art2img/core/encode.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <span>
#include <vector>

#include <stb_image_write.h>

namespace art2img::core {
namespace {

constexpr std::size_t kChannels = 4;

bool validate_view(const RgbaImageView& view) noexcept
{
  return view.valid();
}

std::size_t row_bytes(const RgbaImageView& view) noexcept
{
  return static_cast<std::size_t>(view.width) * kChannels;
}

std::vector<std::uint8_t> make_contiguous_rgba(const RgbaImageView& view)
{
  const auto bytes_per_row = row_bytes(view);
  std::vector<std::uint8_t> buffer(static_cast<std::size_t>(view.height) *
                                   bytes_per_row);
  const auto* src = view.pixels.data();
  auto* dst = buffer.data();
  for (std::uint32_t y = 0; y < view.height; ++y) {
    std::memcpy(dst, src, bytes_per_row);
    src += view.stride;
    dst += bytes_per_row;
  }
  return buffer;
}

std::vector<std::uint8_t> drop_alpha(const std::uint8_t* data,
                                     std::uint32_t width,
                                     std::uint32_t height,
                                     std::size_t stride)
{
  std::vector<std::uint8_t> buffer(static_cast<std::size_t>(width) * height *
                                   3);
  for (std::uint32_t y = 0; y < height; ++y) {
    const auto* src_row = data + static_cast<std::size_t>(y) * stride;
    auto* dst_row = buffer.data() + static_cast<std::size_t>(y) * width * 3;
    for (std::uint32_t x = 0; x < width; ++x) {
      const auto* src_px = src_row + static_cast<std::size_t>(x) * kChannels;
      auto* dst_px = dst_row + static_cast<std::size_t>(x) * 3;
      dst_px[0] = src_px[0];
      dst_px[1] = src_px[1];
      dst_px[2] = src_px[2];
    }
  }
  return buffer;
}

void write_bytes(void* context, void* data, int size)
{
  auto* output = static_cast<std::vector<std::byte>*>(context);
  const auto* begin = static_cast<const std::uint8_t*>(data);
  const auto* end = begin + size;
  output->insert(output->end(), reinterpret_cast<const std::byte*>(begin),
                 reinterpret_cast<const std::byte*>(end));
}

std::expected<std::vector<std::byte>, Error> encode_png(
    const RgbaImageView& view,
    EncoderOptions options)
{
  if (!validate_view(view)) {
    return std::unexpected(
        make_error(errc::encoding_failure, "invalid image view for PNG"));
  }

  const auto expected_stride = row_bytes(view);
  const std::uint8_t* data_ptr = view.pixels.data();
  std::vector<std::uint8_t> owned;
  if (view.stride != expected_stride) {
    owned = make_contiguous_rgba(view);
    data_ptr = owned.data();
  }

  int channels = 4;
  std::vector<std::uint8_t> converted;
  if (options.bit_depth == BitDepth::bpp24) {
    converted = drop_alpha(data_ptr, view.width, view.height, expected_stride);
    data_ptr = converted.data();
    channels = 3;
  }

  std::vector<std::byte> output;
  const int result = stbi_write_png_to_func(
      write_bytes, &output, static_cast<int>(view.width),
      static_cast<int>(view.height), channels, data_ptr, channels * view.width);
  if (result == 0) {
    return std::unexpected(
        make_error(errc::encoding_failure, "failed to encode PNG"));
  }
  return output;
}

std::expected<std::vector<std::byte>, Error> encode_tga(
    const RgbaImageView& view,
    EncoderOptions options)
{
  if (!validate_view(view)) {
    return std::unexpected(
        make_error(errc::encoding_failure, "invalid image view for TGA"));
  }

  const auto expected_stride = row_bytes(view);
  const std::uint8_t* data_ptr = view.pixels.data();
  std::vector<std::uint8_t> owned;
  if (view.stride != expected_stride) {
    owned = make_contiguous_rgba(view);
    data_ptr = owned.data();
  }

  int channels = 4;
  std::vector<std::uint8_t> converted;
  if (options.bit_depth == BitDepth::bpp24) {
    converted = drop_alpha(data_ptr, view.width, view.height, expected_stride);
    data_ptr = converted.data();
    channels = 3;
  }

  std::vector<std::byte> output;
  const int result =
      stbi_write_tga_to_func(write_bytes, &output, static_cast<int>(view.width),
                             static_cast<int>(view.height), channels, data_ptr);
  if (result == 0) {
    return std::unexpected(
        make_error(errc::encoding_failure, "failed to encode TGA"));
  }
  return output;
}

std::expected<std::vector<std::byte>, Error> encode_bmp(
    const RgbaImageView& view,
    EncoderOptions options)
{
  if (!validate_view(view)) {
    return std::unexpected(
        make_error(errc::encoding_failure, "invalid image view for BMP"));
  }

  const auto expected_stride = row_bytes(view);
  const std::uint8_t* data_ptr = view.pixels.data();
  std::vector<std::uint8_t> owned;
  if (view.stride != expected_stride) {
    owned = make_contiguous_rgba(view);
    data_ptr = owned.data();
  }

  int channels = options.bit_depth == BitDepth::bpp24 ? 3 : 4;
  std::vector<std::uint8_t> converted;
  if (channels == 3) {
    converted = drop_alpha(data_ptr, view.width, view.height, expected_stride);
    data_ptr = converted.data();
  }

  std::vector<std::byte> output;
  const int result =
      stbi_write_bmp_to_func(write_bytes, &output, static_cast<int>(view.width),
                             static_cast<int>(view.height), channels, data_ptr);
  if (result == 0) {
    return std::unexpected(
        make_error(errc::encoding_failure, "failed to encode BMP"));
  }
  return output;
}

}  // namespace

std::expected<EncodedImage, Error> encode_image(const RgbaImageView& image,
                                                ImageFormat format,
                                                EncoderOptions options)
{
  if (!image.valid()) {
    return std::unexpected(
        make_error(errc::encoding_failure, "invalid image view"));
  }

  std::expected<std::vector<std::byte>, Error> encoded;
  switch (format) {
    case ImageFormat::png:
      encoded = encode_png(image, options);
      break;
    case ImageFormat::tga:
      encoded = encode_tga(image, options);
      break;
    case ImageFormat::bmp:
      encoded = encode_bmp(image, options);
      break;
  }

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
