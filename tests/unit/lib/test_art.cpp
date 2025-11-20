#include <doctest/doctest.h>
#include <art2img/art.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

TEST_SUITE("art lib module")
{
    TEST_CASE("Parse ART file from memory")
    {
        const auto test_assets_dir = std::filesystem::path{__FILE__}
                                         .parent_path()
                                         .parent_path()
                                         .parent_path() /
                                     "assets";
        const auto art_file_path = test_assets_dir / "TILES000.ART";

        REQUIRE(std::filesystem::exists(art_file_path));

        // Read file into memory
        std::ifstream file(art_file_path, std::ios::binary | std::ios::ate);
        REQUIRE(file.is_open());
        std::streamsize size = file.tellg();
        REQUIRE(size > 0);
        file.seekg(0, std::ios::beg);
        std::vector<char> buffer(size);
        REQUIRE(file.read(buffer.data(), size));

        auto result = art2img::parse_art(std::as_bytes(std::span(buffer)));
        REQUIRE(result.has_value());

        const auto& art = result.value();
        CHECK(!art.tiles.empty());
        
        // Check first tile if available
        if (!art.tiles.empty()) {
            const auto& tile = art.tiles[0];
            CHECK(tile.width > 0);
            CHECK(tile.height > 0);
            CHECK(tile.indices.size() == static_cast<size_t>(tile.width * tile.height));
        }
    }

    TEST_CASE("Parse invalid data")
    {
        // Empty data
        {
            std::vector<std::byte> empty_data;
            auto result = art2img::parse_art(empty_data);
            CHECK(!result.has_value());
            CHECK(result.error().message == "File too small for header");
        }

        // Invalid version
        {
            std::vector<std::byte> data(20, std::byte{0});
            // Set version to 2 (at offset 0)
            data[0] = std::byte{2}; 
            auto result = art2img::parse_art(data);
            CHECK(!result.has_value());
            CHECK(result.error().message == "Invalid version, expected 1");
        }
    }

    TEST_CASE("Render tile")
    {
        art2img::Tile tile;
        tile.width = 2;
        tile.height = 2;
        // Indices: 0, 1, 255, 0
        tile.indices = {std::byte{0}, std::byte{1}, std::byte{255}, std::byte{0}};

        art2img::Palette palette;
        palette.colors.resize(256);
        palette.colors[0] = {255, 0, 0, 255}; // Red
        palette.colors[1] = {0, 255, 0, 255}; // Green
        // 255 is transparent, color doesn't matter much but let's set it
        palette.colors[255] = {0, 0, 255, 255}; // Blue

        auto result = art2img::render_tile(tile, palette);
        REQUIRE(result.has_value());

        const auto& img = result.value();
        CHECK(img.width == 2);
        CHECK(img.height == 2);
        CHECK(img.rgba.size() == 4 * 4);

        // Pixel 0: Red, Alpha 255
        CHECK(img.rgba[0] == std::byte{255});
        CHECK(img.rgba[1] == std::byte{0});
        CHECK(img.rgba[2] == std::byte{0});
        CHECK(img.rgba[3] == std::byte{255});

        // Pixel 1: Green, Alpha 255
        CHECK(img.rgba[4] == std::byte{0});
        CHECK(img.rgba[5] == std::byte{255});
        CHECK(img.rgba[6] == std::byte{0});
        CHECK(img.rgba[7] == std::byte{255});

        // Pixel 2: Blue (from palette), but Alpha 0 (from index 255)
        CHECK(img.rgba[8] == std::byte{0});
        CHECK(img.rgba[9] == std::byte{0});
        CHECK(img.rgba[10] == std::byte{255});
        CHECK(img.rgba[11] == std::byte{0});

        // Pixel 3: Red, Alpha 255
        CHECK(img.rgba[12] == std::byte{255});
        CHECK(img.rgba[13] == std::byte{0});
        CHECK(img.rgba[14] == std::byte{0});
        CHECK(img.rgba[15] == std::byte{255});
    }
}
