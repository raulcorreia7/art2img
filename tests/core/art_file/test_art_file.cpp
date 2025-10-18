#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>

#include "art2img/art_file.hpp"
#include "art2img/exceptions.hpp"
#include "test_helpers.hpp"

TEST_CASE("ArtFile basic functionality - initialization and file loading") {
  SUBCASE("Default construction creates empty art file object") {
    art2img::ArtFile art_file;
    CHECK(!art_file.is_open());
    CHECK_EQ(art_file.data_size(), 0);
    CHECK_EQ(art_file.tiles().size(), 0);
    CHECK_EQ(art_file.header().version, 0);
  }

  SUBCASE("Construction with valid ART file path loads tile data") {
    const auto tiles_path = test_asset_path("TILES000.ART");
    art2img::ArtFile art_file(tiles_path.string());
    CHECK(art_file.is_open());
    CHECK_GT(art_file.tiles().size(), 0);
    CHECK_EQ(art_file.header().version, 1);
    CHECK_EQ(std::filesystem::path(art_file.filename()), tiles_path);
  }

  SUBCASE("Memory-based construction loads ART data from memory buffer") {
    auto art_data = load_test_asset("TILES000.ART");

    art2img::ArtFile art_file;
    REQUIRE(art_file.load(art_data.data(), art_data.size()));

    CHECK(art_file.is_open());
    CHECK_GT(art_file.tiles().size(), 0);
    CHECK_EQ(art_file.header().version, 1);
    CHECK(art_file.has_data());
  }
}

TEST_CASE("ArtFile header validation - version and tile count consistency") {
  auto art_data = load_test_asset("TILES000.ART");

  art2img::ArtFile art_file;
  REQUIRE(art_file.load(art_data.data(), art_data.size()));

  const auto& header = art_file.header();
  CHECK_EQ(header.version, 1);
  CHECK_LT(header.start_tile, header.end_tile);
  CHECK_EQ(header.num_tiles, header.end_tile - header.start_tile + 1);
  CHECK_LE(header.num_tiles, 9216);  // Max tiles as per implementation
}

TEST_CASE("ArtFile tile metadata - dimensions, animation, and emptiness detection") {
  auto art_data = load_test_asset("TILES000.ART");

  art2img::ArtFile art_file;
  REQUIRE(art_file.load(art_data.data(), art_data.size()));

  const auto& tiles = art_file.tiles();
  REQUIRE_GT(tiles.size(), 0);

  SUBCASE("Tile dimensions are within reasonable bounds and size calculation is correct") {
    const auto& tile = tiles[0];

    // Verify tile fields are reasonable
    CHECK_LE(tile.width, 256);
    CHECK_LE(tile.height, 256);

    // Check size calculation
    CHECK_EQ(tile.size(), static_cast<uint32_t>(tile.width) * tile.height);
  }

  SUBCASE("Animation data fields are within valid bit ranges") {
    const auto& tile = tiles[0];

    // Animation data should be within reasonable ranges
    CHECK_LE(tile.anim_frames(), 64);  // 6 bits
    CHECK_LE(tile.anim_type(), 4);     // 2 bits
    CHECK_LE(tile.anim_speed(), 16);   // 4 bits
  }

  SUBCASE("Detection of empty tiles with zero size") {
    // Find an empty tile if it exists
    bool found_empty = false;
    for (const auto& tile : tiles) {
      if (tile.is_empty()) {
        CHECK(tile.size() == 0);
        found_empty = true;
        break;
      }
    }
    static_cast<void>(found_empty);
    // Note: Empty tiles may or may not exist, so we don't REQUIRE this
  }
}

TEST_CASE("ArtFile error handling - invalid files and corrupted data") {
  SUBCASE("Constructor throws exception for non-existent ART file") {
    CHECK_THROWS_AS(art2img::ArtFile("nonexistent.art"), art2img::ArtException);
  }

  SUBCASE("Memory loading fails for insufficient data size") {
    std::vector<uint8_t> invalid_data = {0x00, 0x00, 0x00, 0x00};  // Too short

    art2img::ArtFile art_file;
    CHECK(!art_file.load(invalid_data.data(), invalid_data.size()));
  }

  SUBCASE("Memory loading fails for unsupported ART version") {
    // Create data with wrong version
    std::vector<uint8_t> wrong_version(16, 0);
    wrong_version[0] = 0xFF;  // Invalid version

    art2img::ArtFile art_file;
    CHECK(!art_file.load(wrong_version.data(), wrong_version.size()));
  }
}

TEST_CASE("ArtFile data reading - tile extraction functionality") {
  auto art_data = load_test_asset("TILES000.ART");

  art2img::ArtFile art_file;
  REQUIRE(art_file.load(art_data.data(), art_data.size()));

  const auto& tiles = art_file.tiles();

  SUBCASE("Read tile pixel data and verify consistency between memory and file sources") {
    for (size_t i = 0; i < std::min(tiles.size(), size_t(5)); ++i) {
      if (!tiles[i].is_empty()) {
        std::vector<uint8_t> buffer;
        REQUIRE(art_file.read_tile_data(i, buffer));

        CHECK_EQ(buffer.size(), tiles[i].size());

        // Verify we can read the same tile from file if available
        const auto tiles_path = test_asset_path("TILES000.ART");
        if (std::filesystem::exists(tiles_path)) {
          art2img::ArtFile file_based(tiles_path.string());
          std::vector<uint8_t> file_buffer;
          REQUIRE(file_based.read_tile_data(i, file_buffer));

          CHECK_EQ(buffer.size(), file_buffer.size());
          CHECK(std::equal(buffer.begin(), buffer.end(), file_buffer.begin()));
        }
      }
    }
  }

  SUBCASE("Read operation fails for out-of-range tile index") {
    std::vector<uint8_t> buffer;
    CHECK(!art_file.read_tile_data(tiles.size(), buffer));  // Out of range
  }
}
