#include "cli_app_builder.hpp"

#include <utility>

#include "version.hpp"

namespace art2img {

namespace {
constexpr const char* kDefaultBanner =
    "art2img - Duke Nukem 3D ART File Converter\n"
    "Convert ART files to PNG, TGA, or BMP with transparency support.\n"
    "GPL v2 License - See LICENSE file for complete terms.";

constexpr const char* kDefaultFooter =
    "\nExamples:\n"
    "  art2img tiles.art                  # Convert single ART file\n"
    "  art2img tiles.art -f tga -o out/   # Convert to TGA with output dir\n"
    "  art2img art/ -o images/            # Convert all ART files\n"
    "  art2img tiles.art -p custom.pal    # Use custom palette\n"
    "  art2img tiles.art --no-fix-transparency  # Disable transparency\n"
    "  art2img art/ -m -o game/           # Merge animation data\n"
    "\nFor modders: Use -F for transparency and -m for animation data.";
}  // namespace

CliAppBuilder::CliAppBuilder() : banner_{default_banner()}, footer_{default_footer()} {}

CliAppBuilder& CliAppBuilder::with_banner(std::string banner) {
  if (!banner.empty()) {
    banner_ = std::move(banner);
  }
  return *this;
}

CliAppBuilder& CliAppBuilder::with_footer(std::string footer) {
  if (!footer.empty()) {
    footer_ = std::move(footer);
  }
  return *this;
}

std::unique_ptr<CLI::App> CliAppBuilder::build(CliOptions& options) const {
  auto app = std::make_unique<CLI::App>(banner_, "art2img");
  app->set_version_flag("-v,--version", std::string{"art2img "} + ART2IMG_VERSION);

  app->add_option("ART_FILE|ART_DIRECTORY", options.input_path,
                  "Input ART file or directory containing ART files")
      ->required();

  app->add_option("-o,--output", options.output_dir, "Output directory for converted images")
      ->default_val(".");

  app->add_option("-p,--palette", options.palette_file,
                  "Custom palette file (defaults to built-in Duke Nukem 3D palette)")
      ->type_name("FILE");

  app->add_option("-f,--format", options.format, "Output format: tga, png, or bmp")
      ->default_val("png")
      ->check(CLI::IsMember({"tga", "png", "bmp"}));

  app->add_flag("-F,--fix-transparency,!--no-fix-transparency", options.fix_transparency,
                "Enable magenta transparency fix (default: enabled)");
  app->add_flag("-q,--quiet", options.quiet, "Suppress all non-essential output");
  app->add_flag("-n,--no-anim", options.no_anim, "Skip animation data generation");
  app->add_flag("-m,--merge-anim", options.merge_anim,
                "Merge all animation data into a single file (directory mode)");
  app->add_flag("--parallel,!--no-parallel", options.enable_parallel,
                "Enable parallel tile export (default: enabled)");
  app->add_option("-j,--jobs", options.max_threads,
                  "Maximum number of worker threads to use (0 = auto)")
      ->check(CLI::NonNegativeNumber)
      ->default_val("0");

  app->footer(footer_);

  return app;
}

std::string CliAppBuilder::default_banner() {
  return kDefaultBanner;
}

std::string CliAppBuilder::default_footer() {
  return kDefaultFooter;
}

}  // namespace art2img
