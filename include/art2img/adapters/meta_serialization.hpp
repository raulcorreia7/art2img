#pragma once

#include <expected>
#include <string>

#include "../core/error.hpp"
#include "../core/meta.hpp"

namespace art2img::adapters {

namespace core = ::art2img::core;

std::expected<std::string, core::Error> format_animation_ini(
    const core::ExportManifest& manifest);

std::expected<std::string, core::Error> format_animation_json(
    const core::ExportManifest& manifest);

}  // namespace art2img::adapters
