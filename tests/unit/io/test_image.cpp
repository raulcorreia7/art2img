#include <doctest/doctest.h>
#include "art2img/image.hpp"
#include <vector>

TEST_CASE("Image IO") {
    using namespace art2img;
    
    Image img;
    img.width = 2;
    img.height = 2;
    // 2x2 red image
    // std::byte requires explicit conversion from integer
    img.rgba = {
        Byte{255}, Byte{0}, Byte{0}, Byte{255},  Byte{255}, Byte{0}, Byte{0}, Byte{255},
        Byte{255}, Byte{0}, Byte{0}, Byte{255},  Byte{255}, Byte{0}, Byte{0}, Byte{255}
    };

    SUBCASE("Encode PNG") {
        auto result = encode_png(img);
        CHECK(result.has_value());
        CHECK(result->size() > 0);
    }

    SUBCASE("Encode BMP") {
        auto result = encode_bmp(img);
        CHECK(result.has_value());
        CHECK(result->size() > 0);
    }

    SUBCASE("Encode TGA") {
        auto result = encode_tga(img);
        CHECK(result.has_value());
        CHECK(result->size() > 0);
    }
}
