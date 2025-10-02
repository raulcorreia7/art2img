#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "test_helpers.hpp"
#include "art_file.hpp"
#include "exceptions.hpp"
#include <filesystem>

TEST_CASE("ArtFile basic functionality") {
    SUBCASE("Default construction") {
        art2img::ArtFile art_file;
        CHECK(!art_file.is_open());
        CHECK_EQ(art_file.data_size(), 0);
        CHECK_EQ(art_file.tiles().size(), 0);
        CHECK_EQ(art_file.header().version, 0);
    }

    SUBCASE("File construction") {
        art2img::ArtFile art_file("tests/assets/TILES000.ART");
        CHECK(art_file.is_open());
        CHECK_GT(art_file.tiles().size(), 0);
        CHECK_EQ(art_file.header().version, 1);
        CHECK_EQ(art_file.filename(), "tests/assets/TILES000.ART");
    }

    SUBCASE("Memory-based construction") {
        auto art_data = load_test_asset("TILES000.ART");

        art2img::ArtFile art_file;
        REQUIRE(art_file.load_from_memory(art_data.data(), art_data.size()));

        CHECK(art_file.is_open());
        CHECK_GT(art_file.tiles().size(), 0);
        CHECK_EQ(art_file.header().version, 1);
        CHECK(art_file.has_data());
    }
}

TEST_CASE("ArtFile header validation") {
    auto art_data = load_test_asset("TILES000.ART");

    art2img::ArtFile art_file;
    REQUIRE(art_file.load_from_memory(art_data.data(), art_data.size()));

    const auto& header = art_file.header();
    CHECK_EQ(header.version, 1);
    CHECK_LT(header.start_tile, header.end_tile);
    CHECK_EQ(header.num_tiles, header.end_tile - header.start_tile + 1);
    CHECK_LE(header.num_tiles, 9216); // Max tiles as per implementation
}

TEST_CASE("ArtFile tile metadata") {
    auto art_data = load_test_asset("TILES000.ART");

    art2img::ArtFile art_file;
    REQUIRE(art_file.load_from_memory(art_data.data(), art_data.size()));

    const auto& tiles = art_file.tiles();
    REQUIRE_GT(tiles.size(), 0);

    SUBCASE("Tile structure validation") {
        const auto& tile = tiles[0];

        // Verify tile fields are reasonable
        CHECK_LE(tile.width, 256);
        CHECK_LE(tile.height, 256);

        // Check size calculation
        CHECK_EQ(tile.size(), static_cast<uint32_t>(tile.width) * tile.height);
    }

    SUBCASE("Animation data validation") {
        const auto& tile = tiles[0];

        // Animation data should be within reasonable ranges
        CHECK_LE(tile.anim_frames(), 64); // 6 bits
        CHECK_LE(tile.anim_type(), 4);    // 2 bits
        CHECK_LE(tile.anim_speed(), 16);  // 4 bits
    }

    SUBCASE("Empty tiles") {
        // Find an empty tile if it exists
        bool found_empty = false;
        for (const auto& tile : tiles) {
            if (tile.is_empty()) {
                CHECK(tile.size() == 0);
                found_empty = true;
                break;
            }
        }
        // Note: Empty tiles may or may not exist, so we don't REQUIRE this
    }
}

TEST_CASE("ArtFile error handling") {
    SUBCASE("Invalid file") {
        CHECK_THROWS_AS(art2img::ArtFile("nonexistent.art"), art2img::ArtException);
    }

    SUBCASE("Invalid memory data") {
        std::vector<uint8_t> invalid_data = {0x00, 0x00, 0x00, 0x00}; // Too short

        art2img::ArtFile art_file;
        CHECK(!art_file.load_from_memory(invalid_data.data(), invalid_data.size()));
    }

    SUBCASE("Wrong version") {
        // Create data with wrong version
        std::vector<uint8_t> wrong_version(16, 0);
        wrong_version[0] = 0xFF; // Invalid version

        art2img::ArtFile art_file;
        CHECK(!art_file.load_from_memory(wrong_version.data(), wrong_version.size()));
    }
}

TEST_CASE("ArtFile data reading") {
    auto art_data = load_test_asset("TILES000.ART");

    art2img::ArtFile art_file;
    REQUIRE(art_file.load_from_memory(art_data.data(), art_data.size()));

    const auto& tiles = art_file.tiles();

    SUBCASE("Read tile data") {
        for (size_t i = 0; i < std::min(tiles.size(), size_t(5)); ++i) {
            if (!tiles[i].is_empty()) {
                std::vector<uint8_t> buffer;
                REQUIRE(art_file.read_tile_data_from_memory(i, buffer));

                CHECK_EQ(buffer.size(), tiles[i].size());

                // Verify we can read the same tile from file if available
                if (std::filesystem::exists("tests/assets/TILES000.ART")) {
                    art2img::ArtFile file_based("tests/assets/TILES000.ART");
                    std::vector<uint8_t> file_buffer;
                    REQUIRE(file_based.read_tile_data(i, file_buffer));

                    CHECK_EQ(buffer.size(), file_buffer.size());
                    CHECK(std::equal(buffer.begin(), buffer.end(), file_buffer.begin()));
                }
            }
        }
    }

    SUBCASE("Read invalid tile") {
        std::vector<uint8_t> buffer;
        CHECK(!art_file.read_tile_data_from_memory(tiles.size(), buffer)); // Out of range
    }
}