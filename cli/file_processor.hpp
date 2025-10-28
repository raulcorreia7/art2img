#pragma once

#include <cstddef>
#include <expected>
#include <filesystem>

#include <art2img/core/art.hpp>
#include <art2img/core/error.hpp>
#include <art2img/core/palette.hpp>

#include "config_parser.hpp"
#include "conversion_pipeline.hpp"

namespace art2img::cli {

struct FileProcessingResult {
  std::size_t total_tiles;
  std::size_t failures;
};

std::expected<FileProcessingResult, art2img::core::Error> process_art_file(
    const CliConfig& config,
    art2img::core::ImageFormat format);

}  // namespace art2img::cli