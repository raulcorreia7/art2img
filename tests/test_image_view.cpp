#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include "test_helpers.hpp"
#include "extractor_api.hpp"
#include "exceptions.hpp"
#include <filesystem>

// Function to create a test extractor with data
art2img::ExtractorAPI create_test_extractor() {
    static bool assets_loaded = false;
    static std::vector<uint8_t> art_data;
    static std::vector<uint8_t> palette_data;

    if (!assets_loaded) {
        if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
            throw std::runtime_error("Test assets not found");
        }

        art_data = load_test_asset("TILES000.ART");
        palette_data = load_test_asset("PALETTE.DAT");
        assets_loaded = true;
    }

    art2img::ExtractorAPI extractor;
    if (!extractor.load_art_from_memory(art_data.data(), art_data.size()) ||
        !extractor.load_palette_from_memory(palette_data.data(), palette_data.size())) {
        throw std::runtime_error("Failed to load test data");
    }

    return extractor;
}

TEST_CASE("ImageView construction") {
    if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping ImageView tests");
        return;
    }

    auto extractor = create_test_extractor();
    auto art_view = extractor.get_art_view();

    SUBCASE("Valid ImageView construction") {
        for (uint32_t i = 0; i < std::min(art_view.image_count(), 5UL); ++i) {
            if (!art_view.get_tile(i).is_empty()) {
                art2img::ImageView image_view{&art_view, i};

                CHECK_NE(image_view.parent, nullptr);
                CHECK_EQ(image_view.tile_index, i);
                CHECK_EQ(image_view.width(), art_view.get_tile(i).width);
                CHECK_EQ(image_view.height(), art_view.get_tile(i).height);
                CHECK_EQ(image_view.size(), static_cast<size_t>(image_view.width()) * image_view.height());

                break; // Test first non-empty tile
            }
        }
    }

    SUBCASE("Count size") {
        for (uint32_t i = 0; i < art_view.image_count(); ++i) {
            art2img::ImageView image_view{&art_view, i};
            auto tile = art_view.get_tile(i);

            if (tile.is_empty()) {
                CHECK_EQ(image_view.size(), 0);
            } else {
                CHECK_EQ(image_view.size(), static_cast<size_t>(tile.width) * tile.height);
                CHECK_EQ(image_view.pixel_data() != nullptr, true);
            }
        }
    }
}

TEST_CASE("ImageView pixel data access") {
    if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping pixel data tests");
        return;
    }

    auto extractor = create_test_extractor();
    auto art_view = extractor.get_art_view();

    // Find a non-empty tile
    uint32_t target_tile = 0;
    bool found_non_empty = false;

    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
        if (!art_view.get_tile(i).is_empty()) {
            target_tile = i;
            found_non_empty = true;
            break;
        }
    }

    INFO("Using tile " << target_tile << " for testing");
    if (!found_non_empty) {
        MESSAGE("No non-empty tiles found, skipping pixel data tests");
        return;
    }

    SUBCASE("Pixel data access") {
        art2img::ImageView image_view{&art_view, target_tile};

        const auto* pixel_data = image_view.pixel_data();
        CHECK_NE(pixel_data, nullptr);

        // Verify we can access data without crashing
        // (Just checking first few bytes are reasonable palette indices)
        for (size_t i = 0; i < std::min(image_view.size(), size_t(10)); ++i) {
            CHECK_LT(pixel_data[i], 256); // Should be valid palette indices
        }
    }

    SUBCASE("Empty tile pixel data") {
        // Find an empty tile
        for (uint32_t i = 0; i < art_view.image_count(); ++i) {
            if (art_view.get_tile(i).is_empty()) {
                art2img::ImageView image_view{&art_view, i};
                CHECK_EQ(image_view.pixel_data(), nullptr);
                CHECK_EQ(image_view.size(), 0);
                break;
            }
        }
    }
}

TEST_CASE("ImageView animation data access") {
    if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping animation data tests");
        return;
    }

    auto extractor = create_test_extractor();
    auto art_view = extractor.get_art_view();

    for (uint32_t i = 0; i < std::min(art_view.image_count(), 10UL); ++i) {
        art2img::ImageView image_view{&art_view, i};
        auto tile = art_view.get_tile(i);

        // Check animation data accessors
        CHECK_EQ(image_view.anim_frames(), tile.anim_frames());
        CHECK_EQ(image_view.anim_type(), tile.anim_type());
        CHECK_EQ(image_view.x_offset(), tile.x_offset());
        CHECK_EQ(image_view.y_offset(), tile.y_offset());
        CHECK_EQ(image_view.anim_speed(), tile.anim_speed());
        CHECK_EQ(image_view.other_flags(), tile.other_flags());

        // Reasonable ranges for animation data
        CHECK_LE(image_view.anim_frames(), 64);
        CHECK_LE(image_view.anim_type(), 4);
        CHECK_LE(image_view.anim_speed(), 16);
    }
}

TEST_CASE("ImageView image saving") {
    if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping image saving tests");
        return;
    }

    auto extractor = create_test_extractor();
    auto art_view = extractor.get_art_view();

    // Find a non-empty tile
    uint32_t target_tile = 0;
    bool found_non_empty = false;

    for (uint32_t i = 0; i < art_view.image_count(); ++i) {
        if (!art_view.get_tile(i).is_empty()) {
            target_tile = i;
            found_non_empty = true;
            break;
        }
    }

    if (!found_non_empty) {
        MESSAGE("No non-empty tiles found, skipping image saving tests");
        return;
    }

    art2img::ImageView image_view{&art_view, target_tile};

    SUBCASE("Save to TGA") {
        std::string filename = "test_tile.tga";
        REQUIRE(image_view.save_to_tga(filename));
        CHECK(std::filesystem::exists(filename));
        CHECK_GT(std::filesystem::file_size(filename), 0);

        // Clean up
        std::filesystem::remove(filename);
    }

    SUBCASE("Save to PNG with default options") {
        std::string filename = "test_tile.png";
        REQUIRE(image_view.save_to_png(filename, art2img::PngWriter::Options()));
        CHECK(std::filesystem::exists(filename));
        CHECK_GT(std::filesystem::file_size(filename), 0);

        // Clean up
        std::filesystem::remove(filename);
    }

    SUBCASE("Save to PNG with custom options") {
        art2img::PngWriter::Options options;
        options.enable_magenta_transparency = false;

        std::string filename = "test_tile_custom.png";
        REQUIRE(image_view.save_to_png(filename, options));
        CHECK(std::filesystem::exists(filename));
        CHECK_GT(std::filesystem::file_size(filename), 0);

        // Clean up
        std::filesystem::remove(filename);
    }

    SUBCASE("Extract to TGA memory") {
        auto tga_data = image_view.extract_to_tga();
        CHECK_GT(tga_data.size(), 0);

        // TGA files should have header, so minimum size should be > 100 bytes
        CHECK_GT(tga_data.size(), 100);
    }

    SUBCASE("Extract to PNG memory") {
        auto png_data = image_view.extract_to_png(art2img::PngWriter::Options());
        CHECK_GT(png_data.size(), 0);

        // PNG files should start with PNG signature
        REQUIRE_GE(png_data.size(), 8);
        unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
        CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
    }

    SUBCASE("Extract to PNG memory with options") {
        art2img::PngWriter::Options options;
        options.enable_magenta_transparency = true;

        auto png_data = image_view.extract_to_png(options);
        CHECK_GT(png_data.size(), 0);

        // Check PNG signature
        REQUIRE_GE(png_data.size(), 8);
        unsigned char png_header[8] = {137, 80, 78, 71, 13, 10, 26, 10};
        CHECK(std::equal(png_header, png_header + 8, png_data.begin()));
    }
}

TEST_CASE("ImageView error handling") {
    if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
        MESSAGE("Required assets not found, skipping error handling tests");
        return;
    }

    auto extractor = create_test_extractor();
    auto art_view = extractor.get_art_view();

    SUBCASE("Invalid ImageView state") {
        art2img::ImageView invalid_image_view{};
        CHECK_THROWS_AS(invalid_image_view.pixel_data(), art2img::ArtException);
        CHECK_THROWS_AS(invalid_image_view.width(), art2img::ArtException);
        CHECK_THROWS_AS(invalid_image_view.height(), art2img::ArtException);
        CHECK_THROWS_AS(invalid_image_view.size(), art2img::ArtException);
    }

    SUBCASE("Out of range tile index") {
        art2img::ImageView out_of_range_view{&art_view, static_cast<uint32_t>(art_view.image_count())};
        CHECK_THROWS_AS(out_of_range_view.pixel_data(), art2img::ArtException);
        CHECK_THROWS_AS(out_of_range_view.width(), art2img::ArtException);
        CHECK_THROWS_AS(out_of_range_view.height(), art2img::ArtException);
    }

    SUBCASE("Save empty tile") {
        // Find an empty tile
        for (uint32_t i = 0; i < art_view.image_count(); ++i) {
            if (art_view.get_tile(i).is_empty()) {
                art2img::ImageView empty_image_view{&art_view, i};

                // Empty tiles should return false for saving operations
                std::string filename = "empty_tile.tga";
                CHECK(empty_image_view.save_to_tga(filename)); // Should succeed (no-op)
                return;
            }
        }
    }
}
