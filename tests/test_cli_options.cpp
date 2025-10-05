#include <CLI/CLI.hpp>
#include <doctest/doctest.h>

#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "config.hpp"

namespace {
std::unique_ptr<CLI::App> make_cli_app(CliOptions& options) {
  auto app = std::make_unique<CLI::App>("art2img - Duke Nukem 3D ART File Converter");
  app->set_version_flag("-v,--version", "art2img test");

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

  return app;
}

struct Argv {
  std::vector<std::string> storage;
  std::vector<const char*> argv;

  explicit Argv(std::initializer_list<std::string> args) : storage(args) {
    argv.reserve(storage.size());
    for (const auto& arg : storage) {
      argv.push_back(arg.c_str());
    }
  }

  int argc() const { return static_cast<int>(argv.size()); }
  const char* const* data() const { return argv.data(); }
};
}  // namespace

TEST_CASE("CLI parser assigns defaults with required input") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "tiles.art"};
  REQUIRE_NOTHROW(app->parse(args.argc(), args.data()));

  CHECK(options.input_path == "tiles.art");
  CHECK(options.output_dir == ".");
  CHECK(options.palette_file.empty());
  CHECK(options.format == "png");
  CHECK(options.fix_transparency == true);
  CHECK(options.quiet == false);
  CHECK(options.no_anim == false);
  CHECK(options.merge_anim == false);
}

TEST_CASE("CLI parser accepts fully specified options") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "art_dir", "-o", "out", "-p", "custom.pal", "-f", "tga", "--no-fix-transparency",
            "-q", "-n", "-m"};
  REQUIRE_NOTHROW(app->parse(args.argc(), args.data()));

  CHECK(options.input_path == "art_dir");
  CHECK(options.output_dir == "out");
  CHECK(options.palette_file == "custom.pal");
  CHECK(options.format == "tga");
  CHECK(options.fix_transparency == false);
  CHECK(options.quiet == true);
  CHECK(options.no_anim == true);
  CHECK(options.merge_anim == true);
}

TEST_CASE("CLI parser rejects missing required input") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img"};
  CHECK_THROWS_AS(app->parse(args.argc(), args.data()), CLI::RequiredError);
}

TEST_CASE("CLI parser validates format option") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "tiles.art", "--format", "gif"};
  CHECK_THROWS_AS(app->parse(args.argc(), args.data()), CLI::ValidationError);
}
