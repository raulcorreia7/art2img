#include <doctest/doctest.h>
#include <art2img/api.hpp>
#include <art2img/builders.hpp>

using namespace art2img;

TEST_SUITE("Builder Patterns") {
  TEST_CASE("ConversionOptionsBuilder constructs correctly") {
    auto options = ConversionOptionsBuilder()
                       .apply_lookup(true)
                       .fix_transparency(false)
                       .premultiply_alpha(true)
                       .matte_hygiene(false)
                       .shade_index(5)
                       .build();

    CHECK(options.apply_lookup == true);
    CHECK(options.fix_transparency == false);
    CHECK(options.premultiply_alpha == true);
    CHECK(options.matte_hygiene == false);
    CHECK(options.shade_index == 5);
  }

  TEST_CASE("ExportOptionsBuilder constructs correctly") {
    auto conversion_options = ConversionOptionsBuilder()
                                  .apply_lookup(true)
                                  .fix_transparency(true)
                                  .build();

    auto options = ExportOptionsBuilder()
                       .output_dir("/tmp/output")
                       .format(ImageFormat::png)
                       .organize_by_format(true)
                       .organize_by_art_file(false)
                       .filename_prefix("tile_")
                       .conversion_options(conversion_options)
                       .enable_parallel(true)
                       .max_threads(4)
                       .build();

    CHECK(options.output_dir == "/tmp/output");
    CHECK(options.format == ImageFormat::png);
    CHECK(options.organize_by_format == true);
    CHECK(options.organize_by_art_file == false);
    CHECK(options.filename_prefix == "tile_");
    CHECK(options.conversion_options.apply_lookup == true);
    CHECK(options.conversion_options.fix_transparency == true);
    CHECK(options.enable_parallel == true);
    CHECK(options.max_threads == 4);
  }

  TEST_CASE("ExportOptionsBuilder accepts ConversionOptionsBuilder") {
    auto options = ExportOptionsBuilder()
                       .output_dir("/tmp/output")
                       .format(ImageFormat::tga)
                       .conversion_options(ConversionOptionsBuilder()
                                               .apply_lookup(false)
                                               .fix_transparency(true)
                                               .shade_index(10))
                       .build();

    CHECK(options.output_dir == "/tmp/output");
    CHECK(options.format == ImageFormat::tga);
    CHECK(options.conversion_options.apply_lookup == false);
    CHECK(options.conversion_options.fix_transparency == true);
    CHECK(options.conversion_options.shade_index == 10);
  }

  TEST_CASE("AnimationExportConfigBuilder constructs correctly") {
    auto config = AnimationExportConfigBuilder()
                      .output_dir("/tmp/animations")
                      .base_name("anim_tile")
                      .include_non_animated(true)
                      .generate_ini(false)
                      .ini_filename("animations.ini")
                      .image_format(ImageFormat::bmp)
                      .include_image_references(true)
                      .build();

    CHECK(config.output_dir == "/tmp/animations");
    CHECK(config.base_name == "anim_tile");
    CHECK(config.include_non_animated == true);
    CHECK(config.generate_ini == false);
    CHECK(config.ini_filename == "animations.ini");
    CHECK(config.image_format == ImageFormat::bmp);
    CHECK(config.include_image_references == true);
  }
}