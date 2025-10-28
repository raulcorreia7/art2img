#include <doctest/doctest.h>

#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <filesystem>

TEST_SUITE("art module")
{
  TEST_CASE("Basic art functionality compilation test")
  {
    // This test verifies that the art module can be included and basic types
    // exist
    using art2img::core::ArtArchive;
    using art2img::core::TileMetrics;
    using art2img::core::TileView;

    // Basic compilation test - more comprehensive tests will be added
    // as the implementation progresses
    CHECK(true);
  }

  TEST_CASE("Load ART archive from file")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";

    // Verify test file exists
    REQUIRE(std::filesystem::exists(art_file));

    // Load the ART file
    auto file_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(file_data.has_value());
    CHECK(!file_data->empty());

    // Parse the ART archive
    auto archive = art2img::core::load_art(*file_data);
    REQUIRE(archive.has_value());

    // Verify archive structure
    CHECK(!archive->raw.empty());
    CHECK(!archive->layout.empty());
    // tile_start can be 0 for the first tile in the sequence

    // Check tile count
    auto tile_count = art2img::core::tile_count(*archive);
    CHECK(tile_count > 0);
    CHECK(tile_count == archive->layout.size());
  }

  TEST_CASE("Extract tiles from ART archive")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";

    auto file_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(file_data.has_value());

    auto archive = art2img::core::load_art(*file_data);
    REQUIRE(archive.has_value());

    auto tile_count = art2img::core::tile_count(*archive);
    REQUIRE(tile_count > 0);

    // Test extracting first tile
    auto first_tile = art2img::core::get_tile(*archive, 0);
    REQUIRE(first_tile.has_value());
    CHECK(first_tile->width > 0);
    CHECK(first_tile->height > 0);
    CHECK(first_tile->valid());

    // Verify tile data size matches dimensions
    const auto expected_size = static_cast<std::size_t>(first_tile->width) *
                               static_cast<std::size_t>(first_tile->height);
    CHECK(first_tile->indices.size() >= expected_size);

    // Test extracting last tile
    if (tile_count > 1) {
      auto last_tile = art2img::core::get_tile(*archive, tile_count - 1);
      REQUIRE(last_tile.has_value());
      CHECK(last_tile->width > 0);
      CHECK(last_tile->height > 0);
      CHECK(last_tile->valid());
    }
  }

  TEST_CASE("Invalid tile index handling")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto art_file = test_assets_dir / "TILES000.ART";

    auto file_data = art2img::adapters::read_binary_file(art_file);
    REQUIRE(file_data.has_value());

    auto archive = art2img::core::load_art(*file_data);
    REQUIRE(archive.has_value());

    auto tile_count = art2img::core::tile_count(*archive);

    // Test invalid tile indices
    auto invalid_tile1 = art2img::core::get_tile(*archive, tile_count);
    CHECK(!invalid_tile1.has_value());

    auto invalid_tile2 =
        art2img::core::get_tile(*archive, static_cast<std::size_t>(-1));
    CHECK(!invalid_tile2.has_value());
  }

  TEST_CASE("Load invalid ART file")
  {
    // Test with empty data
    auto empty_archive = art2img::core::load_art(std::span<const std::byte>{});
    CHECK(!empty_archive.has_value());

    // Test with insufficient data
    std::vector<std::byte> small_data(10, std::byte{0});
    auto small_archive = art2img::core::load_art(small_data);
    CHECK(!small_archive.has_value());
  }

  TEST_CASE("Multiple ART files consistency")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";

    // Test loading multiple ART files to ensure consistency
    std::vector<std::string> art_files = {"TILES000.ART", "TILES001.ART",
                                          "TILES002.ART"};

    for (const auto& filename : art_files) {
      const auto art_file = test_assets_dir / filename;
      if (std::filesystem::exists(art_file)) {
        auto file_data = art2img::adapters::read_binary_file(art_file);
        REQUIRE(file_data.has_value());

        auto archive = art2img::core::load_art(*file_data);
        REQUIRE(archive.has_value());

        auto tile_count = art2img::core::tile_count(*archive);
        CHECK(tile_count > 0);
        CHECK(tile_count == archive->layout.size());

        // Test first tile extraction
        auto first_tile = art2img::core::get_tile(*archive, 0);
        if (first_tile.has_value()) {
          CHECK(first_tile->valid());
        }
      }
    }
  }
}