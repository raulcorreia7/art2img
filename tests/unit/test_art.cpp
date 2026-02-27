// Unit tests: ART file format

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

TEST_SUITE("ART") {
    TEST_CASE("ArtFile load empty data") {
        std::vector<std::byte> empty;
        auto result = ArtFile::load(empty);
        CHECK(!result.ok());
        // Empty data returns corrupted_data (too small)
        CHECK(result.code() == error::corrupted_data);
    }

    TEST_CASE("ArtFile load too small") {
        std::vector<std::byte> small(8);
        auto result = ArtFile::load(small);
        CHECK(!result.ok());
        CHECK(result.code() == error::corrupted_data);
    }

    TEST_CASE("ArtFile load wrong version") {
        std::vector<std::byte> data(16);
        data[0] = std::byte{2};  // Version 2
        auto result = ArtFile::load(data);
        CHECK(!result.ok());
        CHECK(result.code() == error::invalid_format);
    }

    TEST_CASE("ArtFile load invalid range") {
        auto data = make_test_art(2, 2, 1);
        // Corrupt: make end < start
        data[12] = std::byte{10};  // end = 10
        data[8] = std::byte{20};   // start = 20
        auto result = ArtFile::load(data);
        CHECK(!result.ok());
        // Invalid range returns corrupted_data
        CHECK(result.code() == error::corrupted_data);
    }

    TEST_CASE("ArtFile load valid single tile") {
        auto data = make_test_art(4, 4, 1);
        auto result = ArtFile::load(data);
        CHECK(result.ok());

        auto& art = result.value();
        CHECK(art.tile_count() == 1);
        CHECK(art.tile_start() == 0);
    }

    TEST_CASE("ArtFile load valid multiple tiles") {
        auto data = make_test_art(2, 2, 5);
        auto result = ArtFile::load(data);
        CHECK(result.ok());

        auto& art = result.value();
        CHECK(art.tile_count() == 5);
    }

    TEST_CASE("ArtFile get_tile valid") {
        auto data = make_test_art(2, 2, 1);
        auto result = ArtFile::load(data);
        REQUIRE(result.ok());

        auto& art = result.value();
        auto tile = art.get_tile(0);
        CHECK(tile.has_value());
        CHECK(tile->width == 2);
        CHECK(tile->height == 2);
        CHECK(tile->count == 4);  // 2x2 pixels
        CHECK(tile->indices != nullptr);
    }

    TEST_CASE("ArtFile get_tile out of bounds") {
        auto data = make_test_art(2, 2, 1);
        auto result = ArtFile::load(data);
        REQUIRE(result.ok());

        auto& art = result.value();
        CHECK(!art.get_tile(1).has_value());
        CHECK(!art.get_tile(100).has_value());
    }

    TEST_CASE("ArtFile get_tile_size") {
        auto data = make_test_art(8, 16, 3);
        auto result = ArtFile::load(data);
        REQUIRE(result.ok());

        auto& art = result.value();

        // Tile 0
        auto size0 = art.get_tile_size(0);
        CHECK(size0.has_value());
        CHECK(size0->width == 8);
        CHECK(size0->height == 16);

        // Tile 1
        auto size1 = art.get_tile_size(1);
        CHECK(size1.has_value());
        CHECK(size1->width == 8);
        CHECK(size1->height == 16);

        // Tile 2
        auto size2 = art.get_tile_size(2);
        CHECK(size2.has_value());
        CHECK(size2->width == 8);
        CHECK(size2->height == 16);
    }

    TEST_CASE("ArtFile get_tile_size out of bounds") {
        auto data = make_test_art(2, 2, 1);
        auto result = ArtFile::load(data);
        REQUIRE(result.ok());

        auto& art = result.value();
        CHECK(!art.get_tile_size(1).has_value());
    }

    TEST_CASE("ArtFile get_anim valid") {
        auto data = make_test_art(2, 2, 1);
        auto result = ArtFile::load(data);
        REQUIRE(result.ok());

        auto& art = result.value();
        auto anim = art.get_anim(0);
        CHECK(anim.has_value());
        CHECK(anim->anim_frames == 0);
        CHECK(anim->anim_speed == 0);
    }

    TEST_CASE("ArtFile get_anim out of bounds") {
        auto data = make_test_art(2, 2, 1);
        auto result = ArtFile::load(data);
        REQUIRE(result.ok());

        auto& art = result.value();
        CHECK(!art.get_anim(1).has_value());
    }

    TEST_CASE("ArtFile from_grp") {
        // Create GRP with ART file inside
        auto grp_data = make_test_grp();
        auto result = GrpFile::load(grp_data);
        REQUIRE(result.ok());

        auto art_result = ArtFile::from_grp(result.value(), "tiles.art");
        CHECK(art_result.ok());
    }

    TEST_CASE("ArtFile from_grp non-existent") {
        auto grp_data = make_test_grp();
        auto result = GrpFile::load(grp_data);
        REQUIRE(result.ok());

        auto art_result = ArtFile::from_grp(result.value(), "missing.art");
        CHECK(!art_result.ok());
        CHECK(art_result.code() == error::not_found);
    }

    TEST_CASE("ArtFile large tile") {
        // Test with 256x256 tile
        auto data = make_test_art(MAX_TILE_DIM, MAX_TILE_DIM, 1);
        auto result = ArtFile::load(data);
        CHECK(result.ok());

        auto& art = result.value();
        auto size = art.get_tile_size(0);
        CHECK(size.has_value());
        CHECK(size->width == MAX_TILE_DIM);
        CHECK(size->height == MAX_TILE_DIM);
    }

    TEST_CASE("ArtFile zero tiles") {
        auto data = make_test_art(0, 0, 0);
        auto result = ArtFile::load(data);
        CHECK(result.ok());
        CHECK(result.value().tile_count() == 0);
    }

    TEST_CASE("ArtFile zero size tile") {
        auto data = make_test_art(0, 0, 1);
        auto result = ArtFile::load(data);
        CHECK(result.ok());

        auto& art = result.value();
        auto tile = art.get_tile(0);
        CHECK(tile.has_value());
        CHECK(tile->width == 0);
        CHECK(tile->height == 0);
        CHECK(tile->count == 0);
    }

}  // TEST_SUITE
