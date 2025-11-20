#include <doctest/doctest.h>
#include <art2img/art.hpp>
#include <filesystem>

TEST_SUITE("art lib module")
{
    TEST_CASE("Load ART file")
    {
        const auto test_assets_dir = std::filesystem::path{__FILE__}
                                         .parent_path()
                                         .parent_path()
                                         .parent_path() /
                                     "assets";
        const auto art_file_path = test_assets_dir / "TILES000.ART";

        REQUIRE(std::filesystem::exists(art_file_path));

        auto result = art2img::load_art(art_file_path);
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

    TEST_CASE("Load non-existent file")
    {
        auto result = art2img::load_art("non_existent_file.art");
        CHECK(!result.has_value());
    }
}
