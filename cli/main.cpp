#include <CLI/CLI.hpp>
#include <art2img/adapters/io.hpp>
#include "config_parser.hpp"
#include "file_processor.hpp"
#include "progress_reporter.hpp"

int main(int argc, const char** argv)
{
  CLI::App app{"Convert Build Engine ART tiles to images"};
  art2img::cli::CliConfig config{};
  int shade = -1;
  bool disable_lookup = false, disable_transparency = false;

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

  if (shade >= 0)
    config.shade_index = static_cast<std::uint8_t>(shade);

  auto format_result = art2img::cli::parse_format(config.format);
  if (!format_result) {
    art2img::cli::report_format_error(format_result.error());
    return 1;
  }

  auto result = art2img::cli::process_art_file(config, *format_result);
  if (!result) {
    std::cerr << result.error().message << '\n';
    return 1;
  }

  art2img::cli::report_completion_summary(*result, config.input_art,
                                          config.output_dir);

  return result->failures > 0 ? 1 : 0;
}