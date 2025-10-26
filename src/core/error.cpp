#include <art2img/core/error.hpp>

#include <sstream>

namespace art2img::core {

const char* error_category::name() const noexcept { return "art2img"; }

std::string error_category::message(int ev) const {
  switch (static_cast<errc>(ev)) {
    case errc::none:
      return "No error";
    case errc::io_failure:
      return "Input/output operation failed";
    case errc::invalid_art:
      return "Invalid or corrupted ART file format";
    case errc::invalid_palette:
      return "Invalid or corrupted palette file format";
    case errc::conversion_failure:
      return "Color conversion or pixel transformation failed";
    case errc::encoding_failure:
      return "Image encoding operation failed";
    case errc::unsupported:
      return "Requested operation or format is not supported";
    case errc::no_animation:
      return "No animation data found in ART file";
    default:
      return "Unknown error";
  }
}

const std::error_category& error_category::instance() {
  static const error_category instance;
  return instance;
}

std::error_code make_error_code(errc e) noexcept {
  return std::error_code(static_cast<int>(e), error_category::instance());
}

std::string format_error_message(const std::string& base_message,
                                 const std::string& context) {
  if (context.empty()) {
    return base_message;
  }
  return base_message + " (" + context + ")";
}

std::string format_file_error(const std::string& base_message,
                              const std::filesystem::path& file_path) {
  return base_message + " [file: " + file_path.string() + "]";
}

std::string format_tile_error(const std::string& base_message,
                              std::size_t tile_index) {
  return base_message + " [tile: " + std::to_string(tile_index) + "]";
}

}  // namespace art2img::core
