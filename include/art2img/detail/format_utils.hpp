#pragma once

#include <art2img/types.hpp>
#include <string>

namespace art2img::detail {

/// @brief Get file extension for the given image format (lowercase)
/// @param format The image format
/// @return File extension string (e.g., "png", "tga", "bmp")
inline std::string get_file_extension(ImageFormat format) noexcept {
  switch (format) {
    case ImageFormat::png:
      return "png";
    case ImageFormat::tga:
      return "tga";
    case ImageFormat::bmp:
      return "bmp";
    default:
      return "bin";
  }
}

/// @brief Get format name string for the given image format (uppercase)
/// @param format The image format
/// @return Format name string (e.g., "PNG", "TGA", "BMP")
inline const char* format_to_string(ImageFormat format) noexcept {
  switch (format) {
    case ImageFormat::png:
      return "PNG";
    case ImageFormat::tga:
      return "TGA";
    case ImageFormat::bmp:
      return "BMP";
    default:
      return "Unknown";
  }
}

}  // namespace art2img::detail