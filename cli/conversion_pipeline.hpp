#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>
#include <span>

#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>
#include <art2img/core/error.hpp>
#include <art2img/core/palette.hpp>

#include "config_parser.hpp"

namespace art2img::cli {

std::expected<void, art2img::core::Error> convert_tile(
    std::size_t index,
    const art2img::core::TileView& tile,
    const std::filesystem::path& output_dir,
    const CliConfig& config,
    art2img::core::PaletteView palette,
    art2img::core::ImageFormat format);

}  // namespace art2img::cli