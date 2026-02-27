// Unit tests: Rendering

#include "../test_common.hpp"

// Undef CLI_BINARY_PATH macro if set by build system to avoid conflict with test_constants.hpp
#ifdef CLI_BINARY_PATH
#undef CLI_BINARY_PATH
#endif
#include "../test_constants.hpp"

#include "doctest.h"

#include <art2img.hpp>

#include <vector>

using namespace art2img;
using namespace art2img::test;
using namespace art2img::test::constants;

TEST_SUITE("Render") {
    TEST_CASE("render basic") {
        // Setup: Create ART with 2x2 tile
        auto art_data = make_test_art(SMALL_TILE_SIZE, SMALL_TILE_SIZE, 1);
        auto art_result = ArtFile::load(art_data);
        REQUIRE(art_result.ok());

        // Setup: Create simple Palette
        std::vector<std::byte> pal_data(PALETTE_BASE_SIZE);
        // Index 0: black
        pal_data[0] = std::byte{0};
        pal_data[1] = std::byte{0};
        pal_data[2] = std::byte{0};
        // Index 1: red
        pal_data[3] = std::byte{63};
        pal_data[4] = std::byte{0};
        pal_data[5] = std::byte{0};
        // Index 2: green
        pal_data[6] = std::byte{0};
        pal_data[7] = std::byte{63};
        pal_data[8] = std::byte{0};

        auto pal_result = Palette::load(pal_data);
        REQUIRE(pal_result.ok());

        auto tile = art_result.value().get_tile(0);
        REQUIRE(tile.has_value());

        auto img_result = render(*tile, pal_result.value());
        CHECK(img_result.ok());

        auto& img = img_result.value();
        CHECK(img.width == SMALL_TILE_SIZE);
        CHECK(img.height == SMALL_TILE_SIZE);
        CHECK(img.pixels.size() ==
              SMALL_TILE_SIZE * SMALL_TILE_SIZE * BYTES_PER_RGBA);  // 4 pixels * 4 channels

        // Check pixels after transpose (ART column-major → PNG row-major)
        // ART stores: [0,1,2,255] as column-major for 2x2:
        //   (0,0)=0, (0,1)=1, (1,0)=2, (1,1)=255
        // PNG expects row-major: (0,0)=0, (1,0)=2, (0,1)=1, (1,1)=255
        // Pixel 0 (index 0): black, opaque
        CHECK(img.pixels[0] == 0);
        CHECK(img.pixels[1] == 0);
        CHECK(img.pixels[2] == 0);
        CHECK(img.pixels[3] == 255);

        // Pixel 1 (index 2): green, opaque (transposed from column 1, row 0)
        CHECK(img.pixels[4] == 0);
        CHECK(img.pixels[5] == 255);
        CHECK(img.pixels[6] == 0);
        CHECK(img.pixels[7] == 255);

        // Pixel 2 (index 1): red, opaque (transposed from column 0, row 1)
        CHECK(img.pixels[8] == 255);
        CHECK(img.pixels[9] == 0);
        CHECK(img.pixels[10] == 0);
        CHECK(img.pixels[11] == 255);

        // Pixel 3 (index 255): transparent
        CHECK(img.pixels[15] == 0);
    }

    TEST_CASE("render with no transparency fix") {
        auto art_data = make_test_art(SMALL_TILE_SIZE, SMALL_TILE_SIZE, 1);
        auto art_result = ArtFile::load(art_data);
        REQUIRE(art_result.ok());

        std::vector<std::byte> pal_data(PALETTE_BASE_SIZE);
        pal_data[0] = std::byte{63};  // White at index 0

        auto pal_result = Palette::load(pal_data);
        REQUIRE(pal_result.ok());

        auto tile = art_result.value().get_tile(0);
        REQUIRE(tile.has_value());

        RenderOptions opts;
        opts.fix_transparency = false;

        auto img_result = render(*tile, pal_result.value(), opts);
        CHECK(img_result.ok());

        // Index 255 should NOT be transparent
        CHECK(img_result.value().pixels[15] == 255);
    }

    TEST_CASE("render with options") {
        auto art_data = make_test_art(SMALL_TILE_SIZE, SMALL_TILE_SIZE, 1);
        auto art_result = ArtFile::load(art_data);
        REQUIRE(art_result.ok());

        // Create Palette with shade tables (32 shades)
        std::vector<std::byte> pal_data(PALETTE_BASE_SIZE +
                                        SHADE_TABLE_COUNT * PALETTE_COLOR_COUNT);
        for (size_t i = 0; i < PALETTE_COLOR_COUNT; ++i) {
            pal_data[i * 3] = std::byte{63};
            pal_data[i * 3 + 1] = std::byte{63};
            pal_data[i * 3 + 2] = std::byte{63};
        }
        // Shade tables: progressive darkening
        for (int shade = 0; shade < static_cast<int>(SHADE_TABLE_COUNT); ++shade) {
            for (int i = 0; i < static_cast<int>(PALETTE_COLOR_COUNT); ++i) {
                uint8_t val = static_cast<uint8_t>(i * (32 - shade) / 32);
                pal_data[PALETTE_BASE_SIZE + shade * PALETTE_COLOR_COUNT + i] = std::byte{val};
            }
        }

        auto pal_result = Palette::load(pal_data);
        REQUIRE(pal_result.ok());

        auto tile = art_result.value().get_tile(0);
        REQUIRE(tile.has_value());

        // Test shade option
        RenderOptions shade_opts;
        shade_opts.shade = SHADE_MID;

        auto shade_result = render(*tile, pal_result.value(), shade_opts);
        CHECK(shade_result.ok());

        // Test lookup option
        RenderOptions lookup_opts;
        lookup_opts.apply_lookup = true;

        auto lookup_result = render(*tile, pal_result.value(), lookup_opts);
        CHECK(lookup_result.ok());
    }

    TEST_CASE("render empty tile fails") {
        auto art_data = make_test_art(0, 0, 1);
        auto art_result = ArtFile::load(art_data);
        REQUIRE(art_result.ok());

        auto pal_result = Palette::load(make_test_palette());
        REQUIRE(pal_result.ok());

        auto tile = art_result.value().get_tile(0);
        REQUIRE(tile.has_value());

        // Render rejects empty tiles
        auto img_result = render(*tile, pal_result.value());
        CHECK(!img_result.ok());
        CHECK(img_result.code() == error::invalid_argument);
    }

    TEST_CASE("render large tile") {
        auto art_data = make_test_art(LARGE_TILE_DIM, LARGE_TILE_DIM, 1);
        auto art_result = ArtFile::load(art_data);
        REQUIRE(art_result.ok());

        auto pal_result = Palette::load(make_test_palette());
        REQUIRE(pal_result.ok());

        auto tile = art_result.value().get_tile(0);
        REQUIRE(tile.has_value());

        auto img_result = render(*tile, pal_result.value());
        CHECK(img_result.ok());
        CHECK(img_result.value().width == LARGE_TILE_DIM);
        CHECK(img_result.value().height == LARGE_TILE_DIM);
        CHECK(img_result.value().pixels.size() == LARGE_TILE_DIM * LARGE_TILE_DIM * BYTES_PER_RGBA);
    }

}  // TEST_SUITE
