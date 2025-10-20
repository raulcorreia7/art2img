#include <doctest/doctest.h>
#include <art2img/detail/format_utils.hpp>
#include <art2img/types.hpp>

using namespace art2img;

TEST_SUITE("Format Utilities") {
  TEST_CASE("get_file_extension returns correct extensions") {
    CHECK(detail::get_file_extension(ImageFormat::png) == "png");
    CHECK(detail::get_file_extension(ImageFormat::tga) == "tga");
    CHECK(detail::get_file_extension(ImageFormat::bmp) == "bmp");
    CHECK(detail::get_file_extension(static_cast<ImageFormat>(99)) == "bin");
  }

  TEST_CASE("format_to_string returns correct format names") {
    CHECK(std::string(detail::format_to_string(ImageFormat::png)) == "PNG");
    CHECK(std::string(detail::format_to_string(ImageFormat::tga)) == "TGA");
    CHECK(std::string(detail::format_to_string(ImageFormat::bmp)) == "BMP");
    CHECK(std::string(detail::format_to_string(static_cast<ImageFormat>(99))) ==
          "Unknown");
  }
}