#pragma once

#include <expected>
#include <string>

#include "../core/error.hpp"
#include "../core/meta.hpp"

namespace art2img::adapters {

enum class animation_format : std::uint8_t { ini = 0, json = 2 };

std::expected<std::string, core::Error> format_animation_ini(
    const core::ExportManifest& manifest);

std::expected<std::string, core::Error> format_animation_json(
    const core::ExportManifest& manifest);

std::expected<std::string, core::Error> format_animation(
    const core::ExportManifest& manifest,
    animation_format format)
{
  decltype(&format_animation_ini) func = nullptr;

  switch (format) {
    case animation_format::ini: {
      func = format_animation_ini;
      break;
    }
    case animation_format::json: {
      func = format_animation_json;
      break;
    }
    default: {
      return std::unexpected(core::make_error(core::errc::animation_format,
                                              "Invalid animation format"));
      break;
    }
  }

  return func(manifest);
}
}  // namespace art2img::adapters
