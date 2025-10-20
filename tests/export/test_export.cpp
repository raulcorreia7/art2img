/// @file test_export.cpp
/// @brief Unit tests for export functionality

#include <art2img/export.hpp>
#include <doctest/doctest.h>
#include <filesystem>
#include <vector>
#include <string>
#include <regex>
#include "../test_helpers.hpp"

namespace {

// Test fixtures
struct TestFixture {
    art2img::ExportOptions options;
    std::filesystem::path temp_dir;

    TestFixture() {
        temp_dir = test_helpers::get_unit_test_dir("export", "test_export");
        test_helpers::ensure_test_output_dir(temp_dir);
        options.output_dir = temp_dir;
        options.format = art2img::ImageFormat::png;
    }

    ~TestFixture() {
        test_helpers::cleanup_test_output_dir(temp_dir);
    }
};

} // anonymous namespace

TEST_SUITE("Export Unit Tests") {

    TEST_CASE_FIXTURE(TestFixture, "ExportOptions default values") {
        art2img::ExportOptions opts;
        CHECK(opts.output_dir == "");
        CHECK(opts.format == art2img::ImageFormat::png);
        CHECK(opts.organize_by_format == false);
        CHECK(opts.organize_by_art_file == false);
        CHECK(opts.filename_prefix == "tile");
    }

    TEST_CASE_FIXTURE(TestFixture, "ExportResult default values") {
        art2img::ExportResult result;
        CHECK(result.total_tiles == 0);
        CHECK(result.exported_tiles == 0);
        CHECK(result.output_files.empty());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_tile - invalid tile") {
        // Create an invalid tile view (empty)
        art2img::TileView invalid_tile;
        art2img::Palette palette;

        auto result = art2img::export_tile(invalid_tile, palette, options);
        REQUIRE(!result.has_value());
        CHECK(result.error().code == art2img::errc::invalid_art);
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_bundle - empty bundle") {
        art2img::ArtData empty_bundle;
        art2img::Palette palette;

        auto result = art2img::export_art_bundle(empty_bundle, palette, options);
        REQUIRE(result.has_value());
        CHECK(result.value().total_tiles == 0);
        CHECK(result.value().exported_tiles == 0);
        CHECK(result.value().output_files.empty());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - empty list") {
        std::vector<std::filesystem::path> empty_files;
        art2img::Palette palette;

        auto result = art2img::export_art_files(empty_files, palette, options);
        REQUIRE(result.has_value());
        CHECK(result.value().total_tiles == 0);
        CHECK(result.value().exported_tiles == 0);
        CHECK(result.value().output_files.empty());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - invalid file path") {
        std::vector<std::filesystem::path> invalid_files = {"nonexistent.art"};
        art2img::Palette palette;

        auto result = art2img::export_art_files(invalid_files, palette, options);
        REQUIRE(result.has_value()); // Should not fail, just skip invalid files
        CHECK(result.value().total_tiles == 0);
        CHECK(result.value().exported_tiles == 0);
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - with organize_by_format") {
        options.organize_by_format = true;
        std::vector<std::filesystem::path> invalid_files = {"nonexistent.art"};
        art2img::Palette palette;

        auto result = art2img::export_art_files(invalid_files, palette, options);
        REQUIRE(result.has_value());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - with organize_by_art_file") {
        options.organize_by_art_file = true;
        std::vector<std::filesystem::path> invalid_files = {"nonexistent.art"};
        art2img::Palette palette;

        auto result = art2img::export_art_files(invalid_files, palette, options);
        REQUIRE(result.has_value());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - custom filename prefix") {
        options.filename_prefix = "custom_tile";
        std::vector<std::filesystem::path> invalid_files = {"nonexistent.art"};
        art2img::Palette palette;

        auto result = art2img::export_art_files(invalid_files, palette, options);
        REQUIRE(result.has_value());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - TGA format") {
        options.format = art2img::ImageFormat::tga;
        std::vector<std::filesystem::path> invalid_files = {"nonexistent.art"};
        art2img::Palette palette;

        auto result = art2img::export_art_files(invalid_files, palette, options);
        REQUIRE(result.has_value());
    }

    TEST_CASE_FIXTURE(TestFixture, "export_art_files - BMP format") {
        options.format = art2img::ImageFormat::bmp;
        std::vector<std::filesystem::path> invalid_files = {"nonexistent.art"};
        art2img::Palette palette;

        auto result = art2img::export_art_files(invalid_files, palette, options);
        REQUIRE(result.has_value());
    }

} // TEST_SUITE