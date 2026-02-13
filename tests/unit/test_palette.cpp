// Unit tests: Palette

#include "../test_common.hpp"

// Undef CLI_BINARY_PATH macro if set by build system to avoid conflict with test_constants.hpp
#ifdef CLI_BINARY_PATH
#undef CLI_BINARY_PATH
#endif
#include "../test_constants.hpp"

#include "doctest.h"

#include <art2img.hpp>

#include <cstring>
#include <vector>

using namespace art2img;
using namespace art2img::test;
using namespace art2img::test::constants;

TEST_SUITE("Palette") {
  TEST_CASE("Palette load too small") {
    std::vector<std::byte> small(100);
    auto result = Palette::load(small);
    CHECK(!result.ok());
    CHECK(result.code() == error::corrupted_data);
  }

  TEST_CASE("Palette load exact size") {
    auto data = make_test_palette();
    CHECK(data.size() == PALETTE_BASE_SIZE);
    auto result = Palette::load(data);
    CHECK(result.ok());
  }

  TEST_CASE("Palette load ignores extra data") {
    // The basic load() function only loads 768 bytes (the Palette)
    // Shade tables require load_extended()
    std::vector<std::byte> data(PALETTE_EXTENDED_SIZE);
    for (size_t i = 0; i < PALETTE_COLOR_COUNT; ++i) {
      data[i * 3] = std::byte{63};
      data[i * 3 + 1] = std::byte{63};
      data[i * 3 + 2] = std::byte{63};
    }
    for (size_t i = PALETTE_BASE_SIZE; i < PALETTE_EXTENDED_SIZE; ++i) {
      data[i] = std::byte{static_cast<uint8_t>(i % 256)};
    }

    auto result = Palette::load(data);
    CHECK(result.ok());
    // Basic load() ignores extra data
    CHECK(result.value().shade_count() == 0);
  }

  TEST_CASE("Palette get_color valid") {
    auto data = make_test_palette();
    auto result = Palette::load(data);
    REQUIRE(result.ok());

    auto& pal = result.value();
    uint8_t rgba[4];

    // Color 0
    CHECK(pal.get_color(0, rgba));
    CHECK(rgba[3] == 255);  // Alpha = 255 (opaque)

    // Color 255 (transparent in default Palette handling)
    CHECK(pal.get_color(TRANSPARENT_COLOR_INDEX, rgba));
    CHECK(rgba[3] == 0);  // Alpha = 0 (transparent)
  }

  TEST_CASE("Palette color scaling") {
    // Test that 6-bit Palette colors are scaled to 8-bit
    std::vector<std::byte> data(PALETTE_BASE_SIZE);
    // Index 0: RGB = 0, 32, 63
    data[0] = std::byte{0};
    data[1] = std::byte{32};
    data[2] = std::byte{63};

    auto result = Palette::load(data);
    REQUIRE(result.ok());

    auto& pal = result.value();
    uint8_t rgba[4];
    CHECK(pal.get_color(0, rgba));

    // Scaling formula: (x << 2) | (x >> 4)
    // 0 -> 0
    // 32 -> (32 << 2) | (32 >> 4) = 128 | 2 = 130
    // 63 -> (63 << 2) | (63 >> 4) = 252 | 3 = 255
    CHECK(rgba[0] == 0);
    CHECK(rgba[1] == 130);  // (32 << 2) | (32 >> 4)
    CHECK(rgba[2] == 255);  // (63 << 2) | (63 >> 4)
  }

}  // TEST_SUITE
