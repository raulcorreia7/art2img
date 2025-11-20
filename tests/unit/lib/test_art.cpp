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
}
