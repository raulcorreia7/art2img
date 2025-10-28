#include "conversion_pipeline.hpp"

#include <format>
#include <span>

#include <art2img/adapters/io.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>

namespace art2img::cli {

std::expected<void, art2img::core::Error> convert_tile(
    std::size_t index,
    const art2img::core::TileView& tile,
    const std::filesystem::path& output_dir,
    const CliConfig& config,
    art2img::core::PaletteView palette,
    art2img::core::ImageFormat format)
{
  art2img::core::ConversionOptions convert_opts{
      .apply_lookup = config.apply_lookup,
      .shade_index = config.shade_index,
      .fix_transparency = config.fix_transparency,
      .premultiply_alpha = config.premultiply_alpha,
      .matte_hygiene = config.sanitize_matte};
  auto image_result =
      art2img::core::palette_to_rgba(tile, palette, convert_opts);
  if (!image_result) {
    return std::unexpected(image_result.error());
  }

  const auto view = art2img::core::make_view(*image_result);
  auto encoded = art2img::core::encode_image(view, format,
                                             art2img::core::EncoderOptions{});
  if (!encoded) {
    return std::unexpected(encoded.error());
  }

  const auto extension = art2img::core::file_extension(format);
  const auto filename = std::format(
      "{}_{:04}.{}", config.input_art.stem().string(), index, extension);
  const auto output_path = output_dir / filename;
  auto bytes =
      std::span<const std::byte>(encoded->bytes.data(), encoded->bytes.size());
  auto write_result = art2img::adapters::write_file(output_path, bytes);
  if (!write_result) {
    return std::unexpected(write_result.error());
  }

  return {};
}

}  // namespace art2img::cli