#include <cstddef>
#include <expected>
#include <filesystem>
#include <format>
#include <iostream>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include <CLI/CLI.hpp>

#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>
#include <art2img/core/palette.hpp>

namespace fs = std::filesystem;

namespace {

// Keep frequently used types
using art2img::core::ConversionOptions;
using art2img::core::EncoderOptions;
using art2img::core::Error;
using art2img::core::ImageFormat;
using art2img::core::PostprocessOptions;

struct CliConfig {
  fs::path input_art;
  fs::path palette_path;
  fs::path output_dir{"."};
  std::string format{"png"};
  bool apply_lookup{true};
  bool fix_transparency{true};
  bool premultiply_alpha{false};
  bool sanitize_matte{false};
  std::optional<std::uint8_t> shade_index{};
};

std::expected<ImageFormat, std::string> parse_format(std::string_view text) {
  if (text == "png") {
    return ImageFormat::png;
  }
  if (text == "tga") {
    return ImageFormat::tga;
  }
  if (text == "bmp") {
    return ImageFormat::bmp;
  }
  return std::unexpected("unsupported format: " + std::string{text});
}

std::expected<void, Error> convert_tile(std::size_t index,
                                        const art2img::core::TileView& tile,
                                        const fs::path& output_dir,
                                        const CliConfig& config,
                                        art2img::core::PaletteView palette,
                                        ImageFormat format) {
  ConversionOptions convert_opts{};
  convert_opts.apply_lookup = config.apply_lookup;
  convert_opts.shade_index = config.shade_index;

  auto image_result =
      art2img::core::palette_to_rgba(tile, palette, convert_opts);
  if (!image_result) {
    return std::unexpected(image_result.error());
  }

  PostprocessOptions post_opts{};
  post_opts.apply_transparency_fix = config.fix_transparency;
  post_opts.premultiply_alpha = config.premultiply_alpha;
  post_opts.sanitize_matte = config.sanitize_matte;
  art2img::core::postprocess_rgba(*image_result, post_opts);

  const auto view = art2img::core::make_view(*image_result);
  auto encoded = art2img::core::encode_image(view, format, EncoderOptions{});
  if (!encoded) {
    return std::unexpected(encoded.error());
  }

  const auto extension = art2img::core::file_extension(format);
  const auto filename = std::format(
      "{}_{:04}.{}", config.input_art.stem().string(), index, extension);
  const auto output_path = config.output_dir / filename;
  auto bytes =
      std::span<const std::byte>(encoded->bytes.data(), encoded->bytes.size());
  auto write_result = art2img::adapters::write_file(output_path, bytes);
  if (!write_result) {
    return write_result;
  }
  return std::expected<void, Error>{};
}

}  // namespace

int main(int argc, const char** argv) {
  CLI::App app{"Convert Build Engine ART tiles to images"};

  CliConfig config{};
  int shade = -1;
  bool disable_lookup = false;
  bool disable_transparency = false;

  app.add_option("-i,--input", config.input_art, "Input ART file")
      ->required()
      ->check(CLI::ExistingFile);
  app.add_option("-p,--palette", config.palette_path, "Palette DAT file")
      ->required()
      ->check(CLI::ExistingFile);
  app.add_option("-o,--output", config.output_dir,
                 "Directory where converted images are written");
  app.add_option("-f,--format", config.format, "Output format (png|tga|bmp)");
  app.add_flag("--no-lookup", disable_lookup, "Disable lookup remapping");
  app.add_flag("--no-transparency", disable_transparency,
               "Skip transparency cleanup");
  app.add_flag("--premultiply", config.premultiply_alpha,
               "Premultiply alpha channel");
  app.add_flag("--matte", config.sanitize_matte,
               "Apply matte hygiene to semi-transparent pixels");
  app.add_option("--shade", shade, "Shade table index to apply (0-255)")
      ->check(CLI::Range(0, 255));

  CLI11_PARSE(app, argc, argv);

  config.apply_lookup = !disable_lookup;
  config.fix_transparency = !disable_transparency;
  if (shade >= 0) {
    config.shade_index = static_cast<std::uint8_t>(shade);
  }

  auto format_result = parse_format(config.format);
  if (!format_result) {
    std::cerr << format_result.error() << '\n';
    return 1;
  }
  const auto format = *format_result;

  auto art_bytes = art2img::adapters::read_binary_file(config.input_art);
  if (!art_bytes) {
    std::cerr << art_bytes.error().message << '\n';
    return 1;
  }

  auto art = art2img::core::load_art(
      std::span<const std::byte>(art_bytes->data(), art_bytes->size()));
  if (!art) {
    std::cerr << art.error().message << '\n';
    return 1;
  }

  auto palette_bytes = art2img::adapters::read_binary_file(config.palette_path);
  if (!palette_bytes) {
    std::cerr << palette_bytes.error().message << '\n';
    return 1;
  }

  auto palette = art2img::core::load_palette(
      std::span<const std::byte>(palette_bytes->data(), palette_bytes->size()));
  if (!palette) {
    std::cerr << palette.error().message << '\n';
    return 1;
  }

  fs::create_directories(config.output_dir);
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
      std::cerr << std::format("Failed to convert tile {}: {}\n", i,
                               result.error().message);
    }
  }

  if (failures > 0) {
    std::cerr << std::format("Completed with {} failures\n", failures);
    return 1;
  }

  std::cout << std::format("Converted {} tiles from {} to {}\n", total,
                           config.input_art.filename().string(),
                           config.output_dir.string());
  return 0;
}
