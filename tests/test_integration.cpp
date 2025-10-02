#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "test_helpers.hpp"
#include "extractor_api.hpp"
#include "exceptions.hpp"
#include <filesystem>
#include <fstream>

TEST_CASE("Integration tests - Full pipeline") {
    if (!std::filesystem::exists("tests/assets/TILES000.ART") ||
        !std::filesystem::exists("tests/assets/PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping integration tests");
        return;
    }

    SUBCASE("Complete extraction workflow") {
        // Load data from file
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        CHECK_EQ(art_data.size() > 0, true);
        CHECK_EQ(palette_data.size(), 768); // Standard palette size

        // Create extractor API and load data
        art2img::ExtractorAPI extractor;
        REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
        REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

        CHECK(extractor.is_art_loaded());
        CHECK(extractor.is_palette_loaded());
        CHECK_GT(extractor.get_tile_count(), 0);

        // Get ArtView for direct access
        auto art_view = extractor.get_art_view();
        CHECK_EQ(art_view.image_count(), extractor.get_tile_count());

        // Extract and verify at least one tile
        auto result = extractor.extract_tile(0);
        REQUIRE(result.success);
        CHECK_GT(result.width, 0);
        CHECK_GT(result.height, 0);
        CHECK_GT(result.image_data.size(), 0);

        // Verify extracted data is valid PNG/TGA
        if (result.format == "png") {
            // Check PNG signature
            REQUIRE_GE(result.image_data.size(), 8);
            unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
            CHECK(std::equal(png_header, png_header + 8, result.image_data.begin()));
        }
    }

    SUBCASE("Memory vs file consistency") {
        // Load from file
        art2img::ExtractorAPI file_extractor;
        REQUIRE_NOTHROW(file_extractor.load_art_file("tests/assets/TILES000.ART"));
        REQUIRE(file_extractor.load_palette_file("tests/assets/PALETTE.DAT"));

        // Load from memory
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        art2img::ExtractorAPI memory_extractor;
        REQUIRE(memory_extractor.load_art_from_memory(art_data.data(), art_data.size()));
        REQUIRE(memory_extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

        // Compare results
        CHECK_EQ(file_extractor.get_tile_count(), memory_extractor.get_tile_count());

        // Extract the first tile and compare
        auto file_result = file_extractor.extract_tile(0);
        auto memory_result = memory_extractor.extract_tile(0);

        REQUIRE(file_result.success);
        REQUIRE(memory_result.success);

        CHECK_EQ(file_result.width, memory_result.width);
        CHECK_EQ(file_result.height, memory_result.height);
        CHECK_EQ(file_result.format, memory_result.format);
        CHECK_EQ(file_result.image_data.size(), memory_result.image_data.size());

        // Data should be identical
        CHECK(std::equal(file_result.image_data.begin(), file_result.image_data.end(),
                          memory_result.image_data.begin()));
    }

    SUBCASE("Batch extraction consistency") {
        art2img::ExtractorAPI extractor;
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
        REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

        // Extract all tiles as PNG
        auto png_results = extractor.extract_all_tiles();
        CHECK_EQ(png_results.size(), extractor.get_tile_count());

        size_t png_success_count = 0;
        size_t png_empty_count = 0;

        for (const auto& result : png_results) {
            CHECK_EQ(result.format, "png");
            if (result.success) {
                png_success_count++;
                if (result.image_data.empty()) {
                    png_empty_count++;
                }
            }
        }

        CHECK_EQ(png_success_count, png_results.size()); // All should succeed

        // Extract all tiles as TGA
        auto tga_results = extractor.extract_all_tiles_tga();
        CHECK_EQ(tga_results.size(), extractor.get_tile_count());

        size_t tga_success_count = 0;
        size_t tga_empty_count = 0;

        for (const auto& result : tga_results) {
            CHECK_EQ(result.format, "tga");
            if (result.success) {
                tga_success_count++;
                if (result.image_data.empty()) {
                    tga_empty_count++;
                }
            }
        }

        CHECK_EQ(tga_success_count, tga_results.size()); // All should succeed

        // Empty tile counts should match
        CHECK_EQ(png_empty_count, tga_empty_count);
    }

    SUBCASE("ImageView and ExtractorAPI consistency") {
        art2img::ExtractorAPI extractor;
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
        REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

        auto art_view = extractor.get_art_view();

        // Compare first few tiles
        for (uint32_t i = 0; i < std::min(art_view.image_count(), 5UL); ++i) {
            if (art_view.get_tile(i).is_empty()) continue;

            // Extract via ExtractorAPI
            auto extractor_result = extractor.extract_tile(i);

            // Extract via ImageView
            art2img::ImageView image_view{&art_view, i};
            auto image_data = image_view.extract_to_png(art2img::PngWriter::Options());

            REQUIRE(extractor_result.success);
            CHECK_EQ(extractor_result.image_data.size(), image_data.size());

            // Data should be identical
            CHECK(std::equal(extractor_result.image_data.begin(), extractor_result.image_data.end(),
                              image_data.begin()));
        }
    }
}

TEST_CASE("Integration tests - Error handling") {
    if (!std::filesystem::exists("tests/assets/TILES000.ART")) {
        MESSAGE("TILES000.ART not found, skipping error handling tests");
        return;
    }

    SUBCASE("Corrupted palette handling") {
        auto art_data = load_test_asset("TILES000.ART");
        std::vector<uint8_t> corrupted_palette = {0x00}; // Too short

        art2img::ExtractorAPI extractor;
        REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
        CHECK(!extractor.load_palette_from_memory(corrupted_palette.data(), corrupted_palette.size()));

        // Should fall back to default palette
        CHECK(extractor.is_palette_loaded());
    }

    SUBCASE("Partial data extraction") {
        // Load valid ART file
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        art2img::ExtractorAPI extractor;
        REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
        REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

        // Try to extract beyond available tiles
        auto result = extractor.extract_tile(extractor.get_tile_count());
        CHECK(!result.success);
        CHECK(!result.error_message.empty());
    }
}

TEST_CASE("Integration tests - Performance and limits") {
    if (!std::filesystem::exists("tests/assets/TILES000.ART")) {
        MESSAGE("TILES000.ART not found, skipping performance tests");
        return;
    }

    SUBCASE("Large batch extraction") {
        art2img::ExtractorAPI extractor;
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
        REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

        // Check we can handle all tiles
        CHECK_GT(extractor.get_tile_count(), 0);

        // Extract random tiles to test performance
        for (uint32_t i = 0; i < 10 && i < extractor.get_tile_count(); ++i) {
            uint32_t tile_index = (i * 17) % extractor.get_tile_count(); // Pseudo-random
            auto result = extractor.extract_tile(tile_index);
            CHECK(result.success);
        }
    }

    SUBCASE("Memory efficiency") {
        auto art_data = load_test_asset("TILES000.ART");
        auto palette_data = load_test_asset("PALETTE.DAT");

        // Create multiple extractors to test memory management
        std::vector<art2img::ExtractorAPI> extractors;
        for (int i = 0; i < 10; ++i) {
            extractors.emplace_back();
            extractors.back().load_art_from_memory(art_data.data(), art_data.size());
            extractors.back().load_palette_from_memory(palette_data.data(), palette_data.size());
        }

        // All should be valid
        for (const auto& extractor : extractors) {
            CHECK(extractor.is_art_loaded());
            CHECK(extractor.is_palette_loaded());
            CHECK_GT(extractor.get_tile_count(), 0);
        }
    }
}

TEST_CASE("Integration tests - Animation data") {
    if (!std::filesystem::exists("tests/assets/TILES000.ART") ||
        !std::filesystem::exists("tests/assets/PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping animation tests");
        return;
    }

    auto art_data = load_test_asset("TILES000.ART");
    auto palette_data = load_test_asset("PALETTE.DAT");

    art2img::ExtractorAPI extractor;
    REQUIRE(extractor.load_art_from_memory(art_data.data(), art_data.size()));
    REQUIRE(extractor.load_palette_from_memory(palette_data.data(), palette_data.size()));

    SUBCASE("Animation data consistency") {
        auto art_view = extractor.get_art_view();

        // Check animation data for several tiles
        bool found_animated = false;

        for (uint32_t i = 0; i < std::min(art_view.image_count(), 50UL); ++i) {
            art2img::ImageView image_view{&art_view, i};
            auto tile = art_view.get_tile(i);

            // Check animation accessors match tile data
            CHECK_EQ(image_view.anim_frames(), tile.anim_frames());
            CHECK_EQ(image_view.anim_type(), tile.anim_type());
            CHECK_EQ(image_view.x_offset(), tile.x_offset());
            CHECK_EQ(image_view.y_offset(), tile.y_offset());
            CHECK_EQ(image_view.anim_speed(), tile.anim_speed());
            CHECK_EQ(image_view.other_flags(), tile.other_flags());

            // Look for animated tiles
            if (tile.anim_frames() > 0 || tile.anim_type() > 0) {
                found_animated = true;

                // Reasonable limits
                CHECK_LE(tile.anim_frames(), 64);
                CHECK_LE(tile.anim_type(), 3); // 0-3 valid types
                CHECK_LE(tile.anim_speed(), 15);
            }
        }

        if (found_animated) {
            MESSAGE("Found animated tiles in test data");
        } else {
            MESSAGE("No animated tiles found in test data");
        }
    }

    SUBCASE("ExtractionResult animation data") {
        auto results = extractor.extract_all_tiles();

        for (uint32_t i = 0; i < std::min(results.size(), 50UL); ++i) {
            const auto& result = results[i];

            if (result.success) {
                // Animation data should be reasonable
                CHECK_LE(result.anim_frames, 64);
                CHECK_LE(result.anim_type, 3);
                CHECK_GE(result.x_offset, -128); // signed int8_t range
                CHECK_LE(result.x_offset, 127);
                CHECK_GE(result.y_offset, -128);
                CHECK_LE(result.y_offset, 127);
                CHECK_LE(result.anim_speed, 15);
            }
        }
    }
}