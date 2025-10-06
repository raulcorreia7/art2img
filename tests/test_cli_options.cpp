#include <doctest/doctest.h>

#include <CLI/CLI.hpp>
#include <initializer_list>
#include <memory>
#include <string>
#include <vector>

#include "cli_app_builder.hpp"
#include "config.hpp"

namespace {
std::unique_ptr<CLI::App> make_cli_app(CliOptions& options) {
  art2img::CliAppBuilder builder;
  return builder.build(options);
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

  int argc() const {
    return static_cast<int>(argv.size());
  }
  const char* const* data() const {
    return argv.data();
  }
};
}  // namespace

TEST_CASE(
    "CLI parser assigns correct default values when only required input parameter is provided") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "tiles.art"};
  REQUIRE_NOTHROW(app->parse(args.argc(), args.data()));

  CHECK(options.input_path == "tiles.art");
  CHECK(options.output_dir == ".");
  CHECK(options.palette_file.empty());
  CHECK(options.format == "png");
  CHECK(options.anim_format == "ini");
  CHECK(options.fix_transparency == true);
  CHECK(options.quiet == false);
  CHECK(options.no_anim == false);
  CHECK(options.merge_anim == false);
  CHECK(options.flat_output == false);
}

TEST_CASE("CLI parser correctly parses all command-line options and flags") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{
      "art2img", "art_dir", "-o", "out", "-p", "custom.pal", "-f", "tga", "--no-fix-transparency",
      "-q",      "-n",      "-m", "--anim-format", "json", "--flat-output"};
  REQUIRE_NOTHROW(app->parse(args.argc(), args.data()));

  CHECK(options.input_path == "art_dir");
  CHECK(options.output_dir == "out");
  CHECK(options.palette_file == "custom.pal");
  CHECK(options.format == "tga");
  CHECK(options.anim_format == "json");
  CHECK(options.fix_transparency == false);
  CHECK(options.quiet == true);
  CHECK(options.no_anim == true);
  CHECK(options.merge_anim == true);
  CHECK(options.flat_output == true);
}

TEST_CASE("CLI parser throws required field error when input path is not provided") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img"};
  CHECK_THROWS_AS(app->parse(args.argc(), args.data()), CLI::RequiredError);
}

TEST_CASE("CLI parser validates image format and rejects unsupported formats") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "tiles.art", "--format", "gif"};
  CHECK_THROWS_AS(app->parse(args.argc(), args.data()), CLI::ValidationError);
}

TEST_CASE("CLI parser correctly parses new animation format and flat output options") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "tiles.art", "--anim-format", "json", "--flat-output"};
  REQUIRE_NOTHROW(app->parse(args.argc(), args.data()));

  CHECK(options.anim_format == "json");
  CHECK(options.flat_output == true);
}

TEST_CASE("CLI parser validates animation format and rejects unsupported animation formats") {
  CliOptions options;
  auto app = make_cli_app(options);

  Argv args{"art2img", "tiles.art", "--anim-format", "xml"};
  CHECK_THROWS_AS(app->parse(args.argc(), args.data()), CLI::ValidationError);
}

TEST_CASE("Config translation correctly handles new animation format and flat output options") {
  CliOptions cli_options;
  cli_options.input_path = "test.art";
  cli_options.anim_format = "json";
  cli_options.flat_output = true;

  auto result = translate_to_processing_options(cli_options);
  REQUIRE(result.success());
  REQUIRE(result.options.has_value());

  CHECK(result.options->anim_format == "json");
  CHECK(result.options->flat_output == true);
  CHECK(result.options->dump_animation == true);  // Should still be true by default

  // Test validation of invalid animation format
  CliOptions invalid_cli_options;
  invalid_cli_options.input_path = "test.art";
  invalid_cli_options.anim_format = "xml";

  auto invalid_result = translate_to_processing_options(invalid_cli_options);
  CHECK_FALSE(invalid_result.success());
  CHECK(invalid_result.error.has_value());
}
