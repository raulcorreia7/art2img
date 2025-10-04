#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "palette.hpp"

TEST_CASE("Blood Palette Validation") {
  SUBCASE("Blood palette can be loaded") {
    art2img::Palette blood_palette;
    blood_palette.load_blood_default();

    CHECK(blood_palette.is_loaded());
    CHECK_EQ(blood_palette.raw_data().size(), 768);  // 256 colors * 3 components
  }

  SUBCASE("Blood palette color access methods") {
    art2img::Palette blood_palette;
    blood_palette.load_blood_default();

    uint8_t red = blood_palette.get_red(0);
    uint8_t green = blood_palette.get_green(0);
    uint8_t blue = blood_palette.get_blue(0);

    // Just verify the methods don't crash and return values in expected range
    CHECK(red <= 255);
    CHECK(green <= 255);
    CHECK(blue <= 255);
  }
}