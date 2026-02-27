// Unit tests: Image encoding

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

TEST_SUITE("Encode") {
    TEST_CASE("encode PNG") {
        // Create simple Image
        Image img;
        img.width = 2;
        img.height = 2;
        img.pixels = {255, 0,   0,   255,  // Red
                      0,   255, 0,   255,  // Green
                      0,   0,   255, 255,  // Blue
                      255, 255, 255, 0};   // Transparent white

        auto result = encode(img, Format::png);
        CHECK(result.ok());
        CHECK(!result.value().empty());

        // PNG magic bytes
        CHECK(result.value()[0] == std::byte{0x89});
        CHECK(result.value()[1] == std::byte{0x50});  // 'P'
        CHECK(result.value()[2] == std::byte{0x4E});  // 'N'
        CHECK(result.value()[3] == std::byte{0x47});  // 'G'
    }

    TEST_CASE("encode TGA") {
        Image img;
        img.width = 2;
        img.height = 2;
        img.pixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0};

        auto result = encode(img, Format::tga);
        CHECK(result.ok());
        CHECK(!result.value().empty());

        // TGA header check
        CHECK(result.value().size() >= 18);  // Minimum header size
    }

    TEST_CASE("encode BMP") {
        Image img;
        img.width = 2;
        img.height = 2;
        img.pixels = {255, 0, 0, 255, 0, 255, 0, 255, 0, 0, 255, 255, 255, 255, 255, 0};

        auto result = encode(img, Format::bmp);
        CHECK(result.ok());
        CHECK(!result.value().empty());

        // BMP magic bytes 'BM'
        CHECK(result.value()[0] == std::byte{0x42});  // 'B'
        CHECK(result.value()[1] == std::byte{0x4D});  // 'M'
    }

    TEST_CASE("encode empty Image fails") {
        Image img;
        img.width = 0;
        img.height = 0;
        img.pixels = {};

        auto result = encode(img, Format::png);
        CHECK(!result.ok());
    }

    TEST_CASE("encode large Image") {
        Image img;
        img.width = MAX_TILE_DIM;
        img.height = MAX_TILE_DIM;
        img.pixels.resize(MAX_TILE_DIM * MAX_TILE_DIM * BYTES_PER_RGBA, 128);

        auto result = encode(img, Format::png);
        CHECK(result.ok());
        CHECK(result.value().size() > 100);  // Should be reasonable size
    }

    TEST_CASE("encode write consistency PNG") {
        auto output_dir = get_test_output_dir("encode_consistency");

        Image img;
        img.width = 4;
        img.height = 4;
        img.pixels.resize(4 * 4 * 4, 200);

        auto enc_result = encode(img, Format::png);
        REQUIRE(enc_result.ok());

        auto out_path = output_dir / "test.png";
        auto write_result = write_file(out_path.string(), enc_result.value());
        CHECK(write_result.ok());
        CHECK(file_exists(out_path.string()));

        // Verify file was written
        CHECK(file_size(out_path.string()) == enc_result.value().size());
    }

    TEST_CASE("encode all formats consistency") {
        Image img;
        img.width = 8;
        img.height = 8;
        img.pixels.resize(8 * 8 * 4, 100);

        auto png = encode(img, Format::png);
        auto tga = encode(img, Format::tga);
        auto bmp = encode(img, Format::bmp);

        CHECK(png.ok());
        CHECK(tga.ok());
        CHECK(bmp.ok());

        // All should produce different but valid data
        CHECK(png.value().size() > 0);
        CHECK(tga.value().size() > 0);
        CHECK(bmp.value().size() > 0);
    }

}  // TEST_SUITE
