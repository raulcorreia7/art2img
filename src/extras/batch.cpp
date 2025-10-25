#include <art2img/extras/batch.hpp>

#include <expected>
#include <utility>

#include <art2img/core/art.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>
#include <art2img/core/palette.hpp>

namespace art2img::extras {

std::expected<BatchResult, core::Error> convert_tiles(
    const BatchRequest& request) {
  if (request.archive == nullptr || request.palette == nullptr) {
    return std::unexpected(core::make_error(core::errc::invalid_art,
                                            "batch request missing data"));
  }

  auto palette_view = core::view_palette(*request.palette);
  BatchResult result{};
  result.images.reserve(request.tiles.size());

  for (std::size_t index : request.tiles) {
    auto tile_view = core::get_tile(*request.archive, index);
    if (!tile_view) {
      return std::unexpected(core::make_error(
          core::errc::invalid_art, "tile index out of range"));
    }

    auto rgba = core::palette_to_rgba(*tile_view, palette_view, request.conversion);
    if (!rgba) {
      return std::unexpected(rgba.error());
    }

    core::postprocess_rgba(*rgba, request.postprocess);
    auto encoded = core::encode_image(core::make_view(*rgba), request.format,
                                      request.encoder);
    if (!encoded) {
      return std::unexpected(encoded.error());
    }

    result.images.push_back(std::move(encoded.value()));
  }

  return result;
}

}  // namespace art2img::extras
