/// @file test_tile_export.cpp
/// @brief Integration tests for tile export functionality
///
/// This test suite covers:
/// - Exporting individual tiles to PNG, TGA, BMP formats
/// - Exporting all tiles from ART files
/// - File I/O verification
/// - Different output directory configurations

#include <art2img/art.hpp>
#include <art2img/convert.hpp>
#include <art2img/encode.hpp>
#include <art2img/palette.hpp>
#include <art2img/io.hpp>
#include <art2img/export.hpp>
#include <doctest/doctest.h>
#include <filesystem>
#include <vector>
#include <iostream>

namespace {

const std::filesystem::path TEST_OUTPUT_DIR = "tests_output";

// Helper function to create output directory
void ensure_output_directory(const std::filesystem::path& dir) {
    std::error_code ec;
    std::filesystem::create_directories(dir, ec);
    if (ec) {
        FAIL("Failed to create output directory: " << ec.message());
    }
}

// Helper function to verify file exists and has content
void verify_output_file(const std::filesystem::path& path) {
    REQUIRE(std::filesystem::exists(path));
    REQUIRE(std::filesystem::file_size(path) > 0);
}

// Helper function to export a single tile using the new export API
std::expected<art2img::ExportResult, art2img::Error> export_single_tile(
    const art2img::TileView& tile,
    const art2img::Palette& palette,
    art2img::ImageFormat format,
    const std::filesystem::path& output_path) {

    art2img::ExportOptions options;
    options.output_dir = output_path.parent_path();
    options.format = format;
    options.filename_prefix = output_path.stem().string();

    auto result = art2img::export_tile(tile, palette, options);
    if (result.has_value()) {
        // The export function generates its own filename, so we need to check
        // what file was actually created and verify it exists
        const auto& exported_files = result.value().output_files;
        if (!exported_files.empty()) {
            // Verify the generated file exists
            if (!std::filesystem::exists(exported_files[0])) {
                return std::unexpected(art2img::Error{art2img::errc::io_failure, 
                    "Exported file does not exist: " + exported_files[0].string()});
            }
        }
    }
    return result;
}

// Helper function to get supported formats
std::vector<art2img::ImageFormat> get_supported_formats() {
    return {art2img::ImageFormat::png, art2img::ImageFormat::tga, art2img::ImageFormat::bmp};
}

} // anonymous namespace

TEST_SUITE("Tile Export Integration") {

    TEST_CASE("Setup - Create output directory") {
        ensure_output_directory(TEST_OUTPUT_DIR);
        CHECK(std::filesystem::exists(TEST_OUTPUT_DIR));
    }

    TEST_CASE("Export single tile - all formats, same output folder") {
        // Load test ART file
        const std::filesystem::path art_path = TEST_ASSET_SOURCE_DIR "/TILES001.ART";
        if (!std::filesystem::exists(art_path)) {
            return; // Skip if file not found
        }

        auto art_result = art2img::load_art_bundle(art_path);
        REQUIRE(art_result.has_value());
        const auto& art_data = art_result.value();

        // Load palette
        const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
        REQUIRE(std::filesystem::exists(palette_path));
        auto palette_result = art2img::load_palette(palette_path);
        REQUIRE(palette_result.has_value());
        const auto& palette = palette_result.value();

        // Get first tile
        auto tile_result = art_data.get_tile(0);
        REQUIRE(tile_result.has_value());
        const auto& tile = tile_result.value();
        REQUIRE(tile.is_valid());

        // Export to all formats in same directory
        const auto formats = get_supported_formats();

        for (const auto format : formats) {
            std::string extension = (format == art2img::ImageFormat::png ? "png" : format == art2img::ImageFormat::tga ? "tga" : "bmp");
            const auto output_path = TEST_OUTPUT_DIR / ("single_tile_same_0." + extension);

            auto export_result = export_single_tile(tile, palette, format, output_path);
            REQUIRE(export_result.has_value());
            REQUIRE(export_result.value().exported_tiles == 1);
            REQUIRE(export_result.value().output_files.size() == 1);
            verify_output_file(export_result.value().output_files[0]);
        }
    }

    TEST_CASE("Export single tile - all formats, different folders per format") {
        // Load test ART file
        const std::filesystem::path art_path = TEST_ASSET_SOURCE_DIR "/TILES004.ART";
        if (!std::filesystem::exists(art_path)) {
            return; // Skip if file not found
        }

        auto art_result = art2img::load_art_bundle(art_path);
        REQUIRE(art_result.has_value());
        const auto& art_data = art_result.value();

        // Load palette
        const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
        REQUIRE(std::filesystem::exists(palette_path));
        auto palette_result = art2img::load_palette(palette_path);
        REQUIRE(palette_result.has_value());
        const auto& palette = palette_result.value();

        // Get first tile
        auto tile_result = art_data.get_tile(0);
        REQUIRE(tile_result.has_value());
        const auto& tile = tile_result.value();
        REQUIRE(tile.is_valid());

        // Export to all formats in different directories
        const auto formats = get_supported_formats();

        for (const auto format : formats) {
            std::string format_name;
            switch (format) {
                case art2img::ImageFormat::png: format_name = "png"; break;
                case art2img::ImageFormat::tga: format_name = "tga"; break;
                case art2img::ImageFormat::bmp: format_name = "bmp"; break;
                default: format_name = "unknown"; break;
            }

            const auto format_dir = TEST_OUTPUT_DIR / format_name;
            ensure_output_directory(format_dir);

            const auto output_path = format_dir / ("single_tile_separate_0." + format_name);

            auto export_result = export_single_tile(tile, palette, format, output_path);
            REQUIRE(export_result.has_value());
            REQUIRE(export_result.value().exported_tiles == 1);
            REQUIRE(export_result.value().output_files.size() == 1);
            verify_output_file(export_result.value().output_files[0]);
        }
    }

    TEST_CASE("Export all tiles - all formats, same output folder") {
        // Load test ART file with multiple tiles
        const std::filesystem::path art_path = TEST_ASSET_SOURCE_DIR "/TILES004.ART";
        if (!std::filesystem::exists(art_path)) {
            return; // Skip if file not found
        }

        auto art_result = art2img::load_art_bundle(art_path);
        REQUIRE(art_result.has_value());
        const auto& art_data = art_result.value();

        // Load palette
        const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
        REQUIRE(std::filesystem::exists(palette_path));
        auto palette_result = art2img::load_palette(palette_path);
        REQUIRE(palette_result.has_value());
        const auto& palette = palette_result.value();

        // Export all tiles to all formats in same directory
        const auto formats = get_supported_formats();
        const std::size_t max_tiles = std::min<std::size_t>(5, art_data.tile_count()); // Limit for test performance

        for (std::size_t tile_idx = 0; tile_idx < max_tiles; ++tile_idx) {
            auto tile_result = art_data.get_tile(tile_idx);
            REQUIRE(tile_result.has_value());
            const auto& tile = tile_result.value();
            if (!tile.is_valid()) {
                std::cout << "Skipping empty tile index " << tile_idx << std::endl;
                continue;
            }

            for (const auto format : formats) {
                std::string extension = (format == art2img::ImageFormat::png ? "png" : format == art2img::ImageFormat::tga ? "tga" : "bmp");
                const auto output_path = TEST_OUTPUT_DIR / ("all_tiles_same_" + std::to_string(tile_idx) + "." + extension);

                auto export_result = export_single_tile(tile, palette, format, output_path);
                REQUIRE(export_result.has_value());
                REQUIRE(export_result.value().exported_tiles == 1);
                REQUIRE(export_result.value().output_files.size() == 1);
                verify_output_file(export_result.value().output_files[0]);
            }
        }
    }

    TEST_CASE("Export all tiles - all formats, different folders per format") {
        // Load test ART file with multiple tiles
        const std::filesystem::path art_path = TEST_ASSET_SOURCE_DIR "/TILES005.ART";
        if (!std::filesystem::exists(art_path)) {
            return; // Skip if file not found
        }

        auto art_result = art2img::load_art_bundle(art_path);
        REQUIRE(art_result.has_value());
        const auto& art_data = art_result.value();

        // Load palette
        const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
        REQUIRE(std::filesystem::exists(palette_path));
        auto palette_result = art2img::load_palette(palette_path);
        REQUIRE(palette_result.has_value());
        const auto& palette = palette_result.value();

        // Export all tiles to all formats in separate directories
        const auto formats = get_supported_formats();
        const std::size_t max_tiles = std::min<std::size_t>(3, art_data.tile_count()); // Limit for test performance

        for (const auto format : formats) {
            std::string format_name;
            switch (format) {
                case art2img::ImageFormat::png: format_name = "png"; break;
                case art2img::ImageFormat::tga: format_name = "tga"; break;
                case art2img::ImageFormat::bmp: format_name = "bmp"; break;
                default: format_name = "unknown"; break;
            }

            const auto format_dir = TEST_OUTPUT_DIR / format_name;
            ensure_output_directory(format_dir);

            for (std::size_t tile_idx = 0; tile_idx < max_tiles; ++tile_idx) {
                auto tile_result = art_data.get_tile(tile_idx);
                REQUIRE(tile_result.has_value());
                const auto& tile = tile_result.value();
                if (!tile.is_valid()) {
                    std::cout << "Skipping empty tile index " << tile_idx << std::endl;
                    continue;
                }

                const auto output_path = format_dir / ("all_tiles_separate_" + std::to_string(tile_idx) + "." + format_name);

                auto export_result = export_single_tile(tile, palette, format, output_path);
                REQUIRE(export_result.has_value());
                REQUIRE(export_result.value().exported_tiles == 1);
                REQUIRE(export_result.value().output_files.size() == 1);
                verify_output_file(export_result.value().output_files[0]);
            }
        }
    }

    TEST_CASE("Error handling - invalid tile export") {
        // Load minimal palette for testing
        const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
        if (!std::filesystem::exists(palette_path)) {
            return; // Skip if palette not found
        }

        auto palette_result = art2img::load_palette(palette_path);
        REQUIRE(palette_result.has_value());
        const auto& palette = palette_result.value();

        // Create a minimal invalid tile view (empty)
        art2img::TileView invalid_tile;

        // Try to export invalid tile
        const auto output_path = TEST_OUTPUT_DIR / "invalid_tile.png";
        auto export_result = export_single_tile(invalid_tile, palette, art2img::ImageFormat::png, output_path);

        // Should fail gracefully
        CHECK(!export_result.has_value());
    }

    TEST_CASE("Export all ART files and tiles - comprehensive dump") {
        // Create comprehensive dump directory
        const auto comprehensive_dump_dir = TEST_OUTPUT_DIR / "comprehensive_dump";
        ensure_output_directory(comprehensive_dump_dir);

        // Load palette once
        const std::filesystem::path palette_path = TEST_ASSET_SOURCE_DIR "/PALETTE.DAT";
        REQUIRE(std::filesystem::exists(palette_path));
        auto palette_result = art2img::load_palette(palette_path);
        REQUIRE(palette_result.has_value());
        const auto& palette = palette_result.value();

        // Get all supported formats
        const auto formats = get_supported_formats();

        // Find all TILES*.ART files
        std::vector<std::filesystem::path> art_files;
        for (const auto& entry : std::filesystem::directory_iterator(TEST_ASSET_SOURCE_DIR)) {
            if (entry.is_regular_file() && entry.path().extension() == ".ART" &&
                entry.path().filename().string().starts_with("TILES")) {
                art_files.push_back(entry.path());
            }
        }

        std::cout << "Found " << art_files.size() << " ART files to process" << std::endl;

        // Process each ART file
        for (const auto& art_path : art_files) {
            std::string art_filename = art_path.filename().string();
            art_filename = art_filename.substr(0, art_filename.find_last_of('.')); // Remove extension

            std::cout << "Processing " << art_filename << std::endl;

            // Load ART file
            auto art_result = art2img::load_art_bundle(art_path);
            REQUIRE(art_result.has_value());
            const auto& art_data = art_result.value();

            std::cout << "  Loaded " << art_data.tile_count() << " tiles" << std::endl;

            // Export all tiles in all formats flat to comprehensive_dump directory
            for (std::size_t tile_idx = 0; tile_idx < art_data.tile_count(); ++tile_idx) {
                auto tile_result = art_data.get_tile(tile_idx);
                REQUIRE(tile_result.has_value());
                const auto& tile = tile_result.value();
                if (!tile.is_valid()) {
                    std::cout << "Skipping empty tile index " << tile_idx << std::endl;
                    continue;
                }

                // Export to each format in the main comprehensive_dump directory
                for (const auto format : formats) {
                    std::string format_name;
                    switch (format) {
                        case art2img::ImageFormat::png: format_name = "png"; break;
                        case art2img::ImageFormat::tga: format_name = "tga"; break;
                        case art2img::ImageFormat::bmp: format_name = "bmp"; break;
                        default: format_name = "unknown"; break;
                    }

                    const auto output_path = comprehensive_dump_dir / (art_filename + "_tile_" + std::to_string(tile_idx) + "." + format_name);

                    auto export_result = export_single_tile(tile, palette, format, output_path);
                    REQUIRE(export_result.has_value());
                    REQUIRE(export_result.value().exported_tiles == 1);
                    REQUIRE(export_result.value().output_files.size() == 1);
                    verify_output_file(export_result.value().output_files[0]);
                }
            }

            std::cout << "  Completed processing " << art_filename << std::endl;
        }

        std::cout << "Comprehensive dump completed successfully" << std::endl;
    }

} // TEST_SUITE
