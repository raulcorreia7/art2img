#include <doctest/doctest.h>
#include "art2img/image.hpp"
#include <filesystem>
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

    std::filesystem::path out_dir = "test_output";
    std::filesystem::create_directories(out_dir);

    SUBCASE("Save PNG") {
        auto path = out_dir / "test.png";
        auto result = save_png(img, path);
        CHECK(result.has_value());
        CHECK(std::filesystem::exists(path));
    }

    SUBCASE("Save BMP") {
        auto path = out_dir / "test.bmp";
        auto result = save_bmp(img, path);
        CHECK(result.has_value());
        CHECK(std::filesystem::exists(path));
    }

    SUBCASE("Save TGA") {
        auto path = out_dir / "test.tga";
        auto result = save_tga(img, path);
        CHECK(result.has_value());
        CHECK(std::filesystem::exists(path));
    }
}
