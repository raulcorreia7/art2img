#pragma once

#include <cstddef>
#include <expected>
#include <vector>

#include "../core/convert.hpp"
#include "../core/encode.hpp"

namespace art2img::extras {

namespace core = ::art2img::core;

struct BatchRequest {
  const core::ArtArchive* archive = nullptr;
  const core::Palette* palette = nullptr;
  std::vector<std::size_t> tiles;
  core::ImageFormat format = core::ImageFormat::png;
  core::ConversionOptions conversion{};
  core::PostprocessOptions postprocess{};
  core::EncoderOptions encoder{};
};

struct BatchResult {
  std::vector<core::EncodedImage> images;
};

std::expected<BatchResult, core::Error> convert_tiles(
    const BatchRequest& request);

}  // namespace art2img::extras
