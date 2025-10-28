#include "file_processor.hpp"

#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/palette.hpp>

#include "progress_reporter.hpp"

namespace art2img::cli {

std::expected<FileProcessingResult, art2img::core::Error> process_art_file(
    const CliConfig& config,
    art2img::core::ImageFormat format)
{
  auto art_bytes = art2img::adapters::read_binary_file(config.input_art);
  if (!art_bytes) {
    return std::unexpected(art_bytes.error());
  }

  auto art = art2img::core::load_art(
      std::span<const std::byte>(art_bytes->data(), art_bytes->size()));
  if (!art) {
    return std::unexpected(art.error());
  }

  auto palette_bytes = art2img::adapters::read_binary_file(config.palette_path);
  if (!palette_bytes) {
    return std::unexpected(palette_bytes.error());
  }

  auto palette = art2img::core::load_palette(
      std::span<const std::byte>(palette_bytes->data(), palette_bytes->size()));
  if (!palette) {
    return std::unexpected(palette.error());
  }

  std::filesystem::create_directories(config.output_dir);
  const auto palette_view = art2img::core::view_palette(*palette);

  const auto total = art2img::core::tile_count(*art);
  std::size_t failures = 0;

  for (std::size_t i = 0; i < total; ++i) {
    auto tile = art2img::core::get_tile(*art, i);
    if (!tile) {
      continue;
    }

    auto result =
        convert_tile(i, *tile, config.output_dir, config, palette_view, format);
    if (!result) {
      ++failures;
      report_conversion_error(i, result.error());
    }
  }

  return FileProcessingResult{total, failures};
}

}  // namespace art2img::cli