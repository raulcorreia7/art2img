/// @file encode.cpp
/// @brief Implementation of image encoding functions using stb library
///
/// This module implements the image encoding functionality as specified in
/// Architecture §4.5 and §9. It handles:
/// - Encoding ImageView to PNG, TGA, and BMP formats using stb writers
/// - Validation of stride/channel metadata before calling stb
/// - Mapping stb failures to errc::encoding_failure
/// - Support for format-specific encoding options
///
/// All functions use std::expected<T, Error> for error handling with proper
/// validation according to Architecture §14 validation rules.

#include <algorithm>
#include <cstring>
#include <vector>

#include <art2img/color_helpers.hpp>
#include <art2img/encode.hpp>

#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include <stb_image_write.h>

namespace art2img {

namespace {
using art2img::types::byte;
using art2img::types::u8;

/// @brief Callback function for stb_image_write to write to vector
void vector_write_func(void* context, void* data, int size) {
  auto* vec = static_cast<std::vector<types::byte>*>(context);
  const auto* byte_data = static_cast<const types::byte*>(data);
  vec->insert(vec->end(), byte_data, byte_data + size);
}

/// @brief Copy RGBA data into a contiguous buffer without padding.
std::vector<u8> copy_rgba_to_contiguous(const ImageView& image) {
  const std::size_t row_bytes =
      static_cast<std::size_t>(image.width) * constants::RGBA_BYTES_PER_PIXEL;
  std::vector<u8> contiguous(static_cast<std::size_t>(image.height) *
                             row_bytes);

  const u8* src_row = image.data.data();
  u8* dst_row = contiguous.data();
  for (u16 y = 0; y < image.height; ++y) {
    std::memcpy(dst_row, src_row, row_bytes);
    src_row += image.stride;
    dst_row += row_bytes;
  }
  return contiguous;
}

/// @brief Flip contiguous image data vertically.
std::vector<u8> flip_image_vertically(const u8* data, u16 width, u16 height,
                                      int channels) {
  const std::size_t row_bytes =
      static_cast<std::size_t>(width) * static_cast<std::size_t>(channels);
  std::vector<u8> flipped(static_cast<std::size_t>(height) * row_bytes);
  for (u16 y = 0; y < height; ++y) {
    const std::size_t dst_row = static_cast<std::size_t>(y) * row_bytes;
    const std::size_t src_row =
        static_cast<std::size_t>(height - 1 - y) * row_bytes;
    std::memcpy(flipped.data() + dst_row, data + src_row, row_bytes);
  }
  return flipped;
}

/// @brief Validate image dimensions and stride
bool validate_image_dimensions(const ImageView& image) {
  // Check basic validity
  if (!image.is_valid()) {
    return false;
  }

  // Check stride consistency
  const std::size_t expected_stride =
      static_cast<std::size_t>(image.width) * constants::RGBA_BYTES_PER_PIXEL;
  if (image.stride != expected_stride) {
    return false;
  }

  // Check data size
  const std::size_t expected_size =
      image.stride * static_cast<std::size_t>(image.height);
  if (image.data.size() < expected_size) {
    return false;
  }

  return true;
}

}  // anonymous namespace

// ============================================================================
// ENCODING FUNCTIONS
// ============================================================================

std::expected<std::vector<byte>, Error> encode_png(const ImageView& image,
                                                   const PngOptions& options) {
  // Validate image before encoding
  if (!validate_image_dimensions(image)) {
    return make_error_expected<std::vector<byte>>(
        errc::encoding_failure,
        "Invalid image dimensions or stride for PNG encoding");
  }

  std::vector<byte> output;

  const int width = static_cast<int>(image.width);
  const int height = static_cast<int>(image.height);
  const std::size_t expected_row_bytes =
      static_cast<std::size_t>(image.width) * constants::RGBA_BYTES_PER_PIXEL;

  std::vector<u8> contiguous_rgba;
  const u8* rgba_data = nullptr;
  if (image.stride == expected_row_bytes) {
    rgba_data = image.data.data();
  } else {
    contiguous_rgba = copy_rgba_to_contiguous(image);
    rgba_data = contiguous_rgba.data();
  }

  int channels = 4;
  int stride = static_cast<int>(image.width) * channels;
  const void* data_ptr = rgba_data;

  // RGBA data is now correct (no need for BGRA conversion)
  // All encoders use consistent RGBA data

  // Convert to grayscale if requested
  std::vector<u8> grayscale_data;
  if (options.convert_to_grayscale) {
    grayscale_data.resize(image.pixel_count());
    for (std::size_t i = 0; i < image.pixel_count(); ++i) {
      const std::size_t offset = i * constants::RGBA_BYTES_PER_PIXEL;
      const u8 r = rgba_data[offset + 0];
      const u8 g = rgba_data[offset + 1];
      const u8 b = rgba_data[offset + 2];
      grayscale_data[i] =
          static_cast<u8>((299u * r + 587u * g + 114u * b) / 1000u);
    }
    data_ptr = grayscale_data.data();
    channels = 1;
    stride = width;  // One byte per pixel
  }

  // Write PNG using stb
  const int result = stbi_write_png_to_func(vector_write_func, &output, width,
                                            height, channels, data_ptr, stride);

  if (result == 0) {
    return make_error_expected<std::vector<byte>>(
        errc::encoding_failure, "PNG encoding failed: stb_image_write error");
  }

  return output;
}

std::expected<std::vector<byte>, Error> encode_tga(const ImageView& image,
                                                   const TgaOptions& options) {
  // Validate image before encoding
  if (!validate_image_dimensions(image)) {
    return make_error_expected<std::vector<byte>>(
        errc::encoding_failure,
        "Invalid image dimensions or stride for TGA encoding");
  }

  std::vector<byte> output;

  int width = static_cast<int>(image.width);
  int height = static_cast<int>(image.height);
  int channels = options.include_alpha ? 4 : 3;

  const std::size_t expected_row_bytes =
      static_cast<std::size_t>(image.width) * constants::RGBA_BYTES_PER_PIXEL;
  std::vector<u8> contiguous_rgba;
  const u8* rgba_data = nullptr;
  if (image.stride == expected_row_bytes) {
    rgba_data = image.data.data();
  } else {
    contiguous_rgba = copy_rgba_to_contiguous(image);
    rgba_data = contiguous_rgba.data();
  }

  // Use RGBA data directly (no conversion needed)
  std::vector<u8> pixel_data;
  if (options.include_alpha) {
    // Copy RGBA data as-is for TGA
    pixel_data.resize(image.pixel_count() * constants::RGBA_BYTES_PER_PIXEL);
    std::memcpy(pixel_data.data(), rgba_data, pixel_data.size());
  } else {
    // Convert RGBA to RGB (drop alpha) for TGA
    pixel_data.resize(image.pixel_count() * 3);
    for (std::size_t i = 0; i < image.pixel_count(); ++i) {
      const std::size_t src_offset = i * constants::RGBA_BYTES_PER_PIXEL;
      const std::size_t dst_offset = i * 3;
      pixel_data[dst_offset + 0] = rgba_data[src_offset + 0];  // R
      pixel_data[dst_offset + 1] = rgba_data[src_offset + 1];  // G
      pixel_data[dst_offset + 2] = rgba_data[src_offset + 2];  // B
    }
  }

  if (options.flip_vertically) {
    auto flipped = flip_image_vertically(pixel_data.data(), image.width,
                                         image.height, channels);
    pixel_data.swap(flipped);
  }

  stbi_write_tga_with_rle = options.use_rle ? 1 : 0;

  // Write TGA using stb
  const int result = stbi_write_tga_to_func(
      vector_write_func, &output, width, height, channels, pixel_data.data());

  if (result == 0) {
    return make_error_expected<std::vector<byte>>(
        errc::encoding_failure, "TGA encoding failed: stb_image_write error");
  }

  return output;
}

std::expected<std::vector<byte>, Error> encode_bmp(const ImageView& image,
                                                   const BmpOptions& options) {
  // Validate image before encoding
  if (!validate_image_dimensions(image)) {
    return make_error_expected<std::vector<byte>>(
        errc::encoding_failure,
        "Invalid image dimensions or stride for BMP encoding");
  }

  std::vector<byte> output;

  int width = static_cast<int>(image.width);
  int height = static_cast<int>(image.height);
  int channels = options.include_alpha ? 4 : 3;

  const std::size_t expected_row_bytes =
      static_cast<std::size_t>(image.width) * constants::RGBA_BYTES_PER_PIXEL;
  std::vector<u8> contiguous_rgba;
  const u8* rgba_data = nullptr;
  if (image.stride == expected_row_bytes) {
    rgba_data = image.data.data();
  } else {
    contiguous_rgba = copy_rgba_to_contiguous(image);
    rgba_data = contiguous_rgba.data();
  }

  // Use RGBA data directly (no conversion needed)
  std::vector<u8> pixel_data;
  if (options.include_alpha) {
    // Copy RGBA data as-is for BMP
    pixel_data.resize(image.pixel_count() * constants::RGBA_BYTES_PER_PIXEL);
    std::memcpy(pixel_data.data(), rgba_data, pixel_data.size());
  } else {
    // Convert RGBA to RGB (drop alpha) for BMP
    pixel_data.resize(image.pixel_count() * 3);
    for (std::size_t i = 0; i < image.pixel_count(); ++i) {
      const std::size_t src_offset = i * constants::RGBA_BYTES_PER_PIXEL;
      const std::size_t dst_offset = i * 3;
      pixel_data[dst_offset + 0] = rgba_data[src_offset + 0];  // R
      pixel_data[dst_offset + 1] = rgba_data[src_offset + 1];  // G
      pixel_data[dst_offset + 2] = rgba_data[src_offset + 2];  // B
    }
  }

  if (options.flip_vertically) {
    auto flipped = flip_image_vertically(pixel_data.data(), image.width,
                                         image.height, channels);
    pixel_data.swap(flipped);
  }

  // Write BMP using stb
  const int result = stbi_write_bmp_to_func(
      vector_write_func, &output, width, height, channels, pixel_data.data());

  if (result == 0) {
    return make_error_expected<std::vector<byte>>(
        errc::encoding_failure, "BMP encoding failed: stb_image_write error");
  }

  return output;
}

std::expected<std::vector<byte>, Error> encode_image(const ImageView& image,
                                                     ImageFormat format,
                                                     EncodeOptions options) {
  // Use default options if std::monostate
  if (std::holds_alternative<std::monostate>(options)) {
    options = get_default_options(format);
  }

  // Dispatch to format-specific encoder
  switch (format) {
    case ImageFormat::png:
      if (auto* png_opts = std::get_if<PngOptions>(&options)) {
        return encode_png(image, *png_opts);
      }
      break;

    case ImageFormat::tga:
      if (auto* tga_opts = std::get_if<TgaOptions>(&options)) {
        return encode_tga(image, *tga_opts);
      }
      break;

    case ImageFormat::bmp:
      if (auto* bmp_opts = std::get_if<BmpOptions>(&options)) {
        return encode_bmp(image, *bmp_opts);
      }
      break;
  }

  return make_error_expected<std::vector<byte>>(
      errc::encoding_failure, "Invalid options for image format");
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

EncodeOptions get_default_options(ImageFormat format) {
  switch (format) {
    case ImageFormat::png:
      return PngOptions{};
    case ImageFormat::tga:
      return TgaOptions{};
    case ImageFormat::bmp:
      return BmpOptions{};
  }
  return std::monostate{};
}

std::expected<std::monostate, Error> validate_image_for_encoding(
    const ImageView& image) {
  if (!validate_image_dimensions(image)) {
    return make_error_expected<std::monostate>(
        errc::encoding_failure,
        "Invalid image dimensions or stride for encoding");
  }
  return make_success();
}

const char* format_to_string(ImageFormat format) {
  switch (format) {
    case ImageFormat::png:
      return "PNG";
    case ImageFormat::tga:
      return "TGA";
    case ImageFormat::bmp:
      return "BMP";
  }
  return "Unknown";
}

}  // namespace art2img
