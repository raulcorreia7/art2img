#pragma once

#include <string>
#include <utility>

#include <art2img/error.hpp>

namespace art2img::core {

using ::art2img::errc;
using ::art2img::Error;
using ::art2img::error_category;
using ::art2img::format_error_message;
using ::art2img::format_file_error;
using ::art2img::format_tile_error;
using ::art2img::make_error_code;
using ::art2img::make_error_expected;
using ::art2img::make_success;

inline Error make_error(errc code, std::string message) {
  return Error(code, std::move(message));
}

}  // namespace art2img::core
