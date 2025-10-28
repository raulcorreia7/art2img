#include "config_parser.hpp"

#include <art2img/core/encode.hpp>

namespace art2img::cli {

std::expected<art2img::core::ImageFormat, std::string> parse_format(
    std::string_view text)
{
  if (text == "png") {
    return art2img::core::ImageFormat::png;
  }

  if (text == "tga") {
    return art2img::core::ImageFormat::tga;
  }

  if (text == "bmp") {
    return art2img::core::ImageFormat::bmp;
  }

  return std::unexpected("unsupported format: " + std::string{text});
}

}  // namespace art2img::cli