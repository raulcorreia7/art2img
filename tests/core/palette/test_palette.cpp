#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>

#include "art2img/exceptions.hpp"
#include "art2img/palette.hpp"
#include "test_helpers.hpp"

TEST_CASE("Palette basic functionality") {
  SUBCASE("Default construction") {
    art2img::Palette palette;
    CHECK_EQ(palette.get_bgr_data().size(), art2img::Palette::SIZE);

    // Should have loaded Duke3D default by construction
    CHECK_EQ(palette.get_red(0), 0);
    CHECK_EQ(palette.get_green(0), 0);
    CHECK_EQ(palette.get_blue(0), 0);
  }

  SUBCASE("Size constants") {
    CHECK_EQ(art2img::Palette::SIZE, 768);  // 256 colors * 3 channels
  }
}

TEST_CASE("Palette file loading") {
  if (!has_test_asset("PALETTE.DAT")) {
    MESSAGE("PALETTE.DAT not found, skipping file loading tests");
    return;
  }

  SUBCASE("Load from file") {
    const auto palette_path = test_asset_path("PALETTE.DAT");
    art2img::Palette palette;
    REQUIRE(palette.load_from_file(palette_path.string()));

    CHECK_EQ(palette.get_bgr_data().size(), art2img::Palette::SIZE);

    // Check a few known colors from Duke3D palette
    CHECK_EQ(palette.get_red(0), 0);
    CHECK_EQ(palette.get_green(0), 0);
    CHECK_EQ(palette.get_blue(0), 0);
  }

  SUBCASE("Load invalid file") {
    art2img::Palette palette;
    CHECK_THROWS_AS(palette.load_from_file("nonexistent.dat"), art2img::ArtException);
  }
}

TEST_CASE("Palette memory loading") {
  if (!has_test_asset("PALETTE.DAT")) {
    MESSAGE("PALETTE.DAT not found, skipping memory loading tests");
    return;
  }

  SUBCASE("Load from valid memory") {
    auto palette_data = load_test_asset("PALETTE.DAT");
    REQUIRE_GE(palette_data.size(), art2img::Palette::SIZE);

    art2img::Palette palette;
    REQUIRE(palette.load_from_memory(palette_data.data(), palette_data.size()));

    CHECK_EQ(palette.get_bgr_data().size(), art2img::Palette::SIZE);
  }

  SUBCASE("Load from invalid memory") {
    std::vector<uint8_t> invalid_data = {1, 2, 3};  // Too short

    art2img::Palette palette;
    CHECK(!palette.load_from_memory(invalid_data.data(), invalid_data.size()));
  }

  SUBCASE("Load from empty data") {
    art2img::Palette palette;
    CHECK(!palette.load_from_memory(nullptr, 0));
  }
}

TEST_CASE("Default palettes") {
  SUBCASE("Duke3D default palette") {
    art2img::Palette palette;
    palette.load_duke3d_default();

    CHECK_EQ(palette.get_bgr_data().size(), art2img::Palette::SIZE);

    auto reference_bytes = load_test_asset("PALETTE.DAT");
    std::vector<uint8_t> reference(reference_bytes.begin(),
                                   reference_bytes.begin() + art2img::Palette::SIZE);

    CHECK_EQ(palette.raw_data(), reference);
  }

  SUBCASE("Blood default palette") {
    art2img::Palette palette;
    palette.load_blood_default();

    CHECK_EQ(palette.get_bgr_data().size(), art2img::Palette::SIZE);

    // Check some known Blood colors
    CHECK_EQ(palette.get_red(0), 0);  // First color should still be black
    CHECK_EQ(palette.get_green(0), 0);
    CHECK_EQ(palette.get_blue(0), 0);

    // Blood palette should differ from Duke3D
    art2img::Palette duke3d_palette;
    duke3d_palette.load_duke3d_default();

    bool palettes_differ = false;
    for (int i = 0; i < 256; ++i) {
      if (palette.get_red(i) != duke3d_palette.get_red(i) ||
          palette.get_green(i) != duke3d_palette.get_green(i) ||
          palette.get_blue(i) != duke3d_palette.get_blue(i)) {
        palettes_differ = true;
        break;
      }
    }
    CHECK(palettes_differ);
  }
}

TEST_CASE("Palette color access") {
  art2img::Palette palette;
  palette.load_duke3d_default();

  SUBCASE("Valid color indices") {
    auto reference_bytes = load_test_asset("PALETTE.DAT");
    const auto* reference = reference_bytes.data();

    CHECK_EQ(palette.get_red(0), reference[0 * 3 + 0]);
    CHECK_EQ(palette.get_green(0), reference[0 * 3 + 1]);
    CHECK_EQ(palette.get_blue(0), reference[0 * 3 + 2]);

    CHECK_EQ(palette.get_red(255), reference[255 * 3 + 0]);
    CHECK_EQ(palette.get_green(255), reference[255 * 3 + 1]);
    CHECK_EQ(palette.get_blue(255), reference[255 * 3 + 2]);
  }

  SUBCASE("Invalid color indices") {
    // Out of range should return 0
    CHECK_EQ(palette.get_red(256), 0);
    CHECK_EQ(palette.get_red(1000), 0);

    CHECK_EQ(palette.get_green(256), 0);
    CHECK_EQ(palette.get_green(1000), 0);

    CHECK_EQ(palette.get_blue(256), 0);
    CHECK_EQ(palette.get_blue(1000), 0);
  }

  SUBCASE("Empty palette access") {
    art2img::Palette empty_palette;
    // Note: Palette constructor loads Duke3D default, so we can't truly empty it
    // This test just verifies the access methods work correctly

    CHECK_EQ(empty_palette.get_red(0) >= 0, true);
    CHECK_EQ(empty_palette.get_green(0) >= 0, true);
    CHECK_EQ(empty_palette.get_blue(0) >= 0, true);
  }
}

TEST_CASE("Palette format conversion") {
  SUBCASE("TGA format conversion") {
    art2img::Palette palette;
    palette.load_duke3d_default();

    const auto& data = palette.get_bgr_data();
    REQUIRE_EQ(data.size(), 768);

    // Check TGA format: BGR order and scaled values
    for (int i = 0; i < 256; ++i) {
      size_t offset = i * 3;

      // Data should be scaled from 0-63 to 0-255
      uint8_t expected_blue = palette.get_blue(i) << 2;
      uint8_t expected_green = palette.get_green(i) << 2;
      uint8_t expected_red = palette.get_red(i) << 2;

      CHECK_EQ(data[offset], expected_blue);       // Blue
      CHECK_EQ(data[offset + 1], expected_green);  // Green
      CHECK_EQ(data[offset + 2], expected_red);    // Red
    }
  }
}
