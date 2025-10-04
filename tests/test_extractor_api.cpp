#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>

#include "exceptions.hpp"
#include "extractor_api.hpp"
#include "test_helpers.hpp"

TEST_CASE("ExtractorAPI construction") {
  SUBCASE("Default construction") {
    art2img::ExtractorAPI extractor;

    CHECK(!extractor.is_art_loaded());
    CHECK(extractor.is_palette_loaded());  // Default palette should be loaded
    CHECK_EQ(extractor.get_tile_count(), 0);
  }

  SUBCASE("Multiple instances") {
    art2img::ExtractorAPI extractor1;
    art2img::ExtractorAPI extractor2;

    CHECK(extractor1.is_palette_loaded());
    CHECK(extractor2.is_palette_loaded());
    CHECK_EQ(extractor1.get_tile_count(), 0);
    CHECK_EQ(extractor2.get_tile_count(), 0);
  }
}

TEST_CASE("ExtractorAPI ART file loading") {
  if (!has_test_asset("TILES000.ART")) {
    MESSAGE("TILES000.ART not found, skipping ART loading tests");
    return;
  }

  SUBCASE("Load ART from file") {
    art2img::ExtractorAPI extractor;
    const auto art_path = test_asset_path("TILES000.ART");

    CHECK_NOTHROW(extractor.load_art_file(art_path.string()));
    CHECK(extractor.is_art_loaded());
    CHECK_GT(extractor.get_tile_count(), 0);
  }

  SUBCASE("Load invalid ART file") {
    art2img::ExtractorAPI extractor;

    CHECK_THROWS_AS(extractor.load_art_file("nonexistent.art"), art2img::ArtException);
    CHECK(!extractor.is_art_loaded());
  }

  SUBCASE("Load ART from memory") {
    auto art_data = load_test_asset("TILES000.ART");
    art2img::ExtractorAPI extractor;

    REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
    CHECK(extractor.is_art_loaded());
    CHECK_GT(extractor.get_tile_count(), 0);
  }

  SUBCASE("Load invalid ART from memory") {
    art2img::ExtractorAPI extractor;

    CHECK(!extractor.load_art_from_memory(nullptr, 0));
    CHECK(!extractor.load_art_from_memory(reinterpret_cast<const uint8_t*>("invalid"), 7));
    CHECK(!extractor.is_art_loaded());
  }
}

TEST_CASE("ExtractorAPI palette loading") {
  if (!has_test_asset("PALETTE.DAT")) {
    MESSAGE("PALETTE.DAT not found, skipping palette loading tests");
    return;
  }

  SUBCASE("Load palette from file") {
    art2img::ExtractorAPI extractor;
    const auto palette_path = test_asset_path("PALETTE.DAT");

    CHECK(extractor.load_palette_file(palette_path.string()));
    CHECK(extractor.is_palette_loaded());
  }

  SUBCASE("Load palette from memory") {
    auto palette_data = load_test_asset("PALETTE.DAT");
    art2img::ExtractorAPI extractor;

    REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));
    CHECK(extractor.is_palette_loaded());
  }

  SUBCASE("Invalid palette loading") {
    art2img::ExtractorAPI extractor;

    CHECK_THROWS_AS(extractor.load_palette_file("nonexistent.dat"), art2img::ArtException);
    CHECK(!extractor.load_palette_from_memory(nullptr, 0));
    CHECK(!extractor.load_palette_from_memory(reinterpret_cast<const uint8_t*>("invalid"), 7));
  }
}

TEST_CASE("ExtractorAPI default palettes") {
  art2img::ExtractorAPI extractor;

  SUBCASE("Duke3D default palette") {
    extractor.set_duke3d_default_palette();
    CHECK(extractor.is_palette_loaded());
  }

  SUBCASE("Blood default palette") {
    extractor.set_blood_default_palette();
    CHECK(extractor.is_palette_loaded());
  }
}

TEST_CASE("ExtractorAPI tile extraction") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping extraction tests");
    return;
  }

  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("Extract single tile to PNG") {
    auto result = extractor.extract_tile_png(0);

    CHECK(result.success);
    CHECK_EQ(result.width > 0, true);
    CHECK_EQ(result.height > 0, true);
    CHECK_EQ(result.format, "png");
    CHECK_GT(result.image_data.size(), 0);
  }

  SUBCASE("Extract single tile to TGA") {
    auto result = extractor.extract_tile_tga(0, art2img::ImageWriter::Options());

    CHECK(result.success);
    CHECK_EQ(result.width > 0, true);
    CHECK_EQ(result.height > 0, true);
    CHECK_EQ(result.format, "tga");
    CHECK_GT(result.image_data.size(), 0);
  }

  SUBCASE("Extract empty tile") {
    // Find an empty tile
    uint32_t empty_tile_index = UINT32_MAX;
    auto art_view = extractor.get_art_view();

    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
      if (art_view.get_tile(i).is_empty()) {
        empty_tile_index = i;
        break;
      }
    }

    if (empty_tile_index != UINT32_MAX) {
      auto result = extractor.extract_tile_png(empty_tile_index);
      CHECK(result.success);  // Empty tiles should succeed but return no data
    }
  }

  SUBCASE("Extract invalid tile index") {
    auto result = extractor.extract_tile_png(extractor.get_tile_count());
    CHECK(!result.success);
  }
}

TEST_CASE("ExtractorAPI batch extraction") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping batch extraction tests");
    return;
  }

  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("Extract all tiles to PNG") {
    auto results = extractor.extract_all_tiles_png();
    CHECK_EQ(results.size(), extractor.get_tile_count());

    size_t success_count = 0;
    size_t empty_count = 0;

    for (const auto& result : results) {
      if (result.success) {
        success_count++;
        if (result.image_data.empty()) {
          empty_count++;
        }
      }
    }

    CHECK_GT(success_count, 0);
    // All extractions should succeed
    CHECK_EQ(success_count, results.size());
  }

  SUBCASE("Extract all tiles to TGA") {
    auto results = extractor.extract_all_tiles_tga(art2img::ImageWriter::Options());
    CHECK_EQ(results.size(), extractor.get_tile_count());

    size_t success_count = 0;
    for (const auto& result : results) {
      if (result.success) {
        success_count++;
        CHECK_EQ(result.format, "tga");
      }
    }

    CHECK_GT(success_count, 0);
    CHECK_EQ(success_count, results.size());
  }
}

TEST_CASE("ExtractorAPI ArtView access") {
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping ArtView tests");
    return;
  }

  auto art_data = load_test_asset("TILES000.ART");
  auto palette_data = load_test_asset("PALETTE.DAT");

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

  SUBCASE("Get ArtView") {
    auto art_view = extractor.get_art_view();

    CHECK_NE(art_view.art_data, nullptr);
    CHECK_GT(art_view.art_size, 0);
    CHECK_NE(art_view.palette, nullptr);
    CHECK_GT(art_view.image_count(), 0);
    CHECK_EQ(art_view.image_count(), extractor.get_tile_count());
  }

  SUBCASE("ArtView error without loaded data") {
    art2img::ExtractorAPI empty_extractor;
    CHECK_THROWS_AS(empty_extractor.get_art_view(), art2img::ArtException);
  }
}
