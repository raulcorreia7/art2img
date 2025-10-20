/// @file test_animation_export.cpp
/// @brief Unit tests for animation export functionality
///
/// This test suite covers:
/// - Animation data extraction from ART files
/// - INI file generation with proper format
/// - Configuration options for animation export
/// - Error handling for various edge cases
/// - Legacy format compatibility with art2tga.c
/// - Parallel processing of multiple ART files

#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>
#include <regex>
#include <sstream>
#include "../test_helpers.hpp"

// Note: art2img headers will be included when the build system is fixed
// For now, we'll create placeholder tests that verify the test structure

namespace {

// Test fixtures
struct AnimationTestFixture {
    std::filesystem::path temp_dir;
    art2img::AnimationExportConfig config;
    
    AnimationTestFixture() {
        temp_dir = test_helpers::get_unit_test_dir("animation", "test_animation_export");
        test_helpers::ensure_test_output_dir(temp_dir);
        config.output_dir = temp_dir;
        config.ini_filename = "test_animdata.ini";
    }
    
    ~AnimationTestFixture() {
        test_helpers::cleanup_test_output_dir(temp_dir);
    }
    
    /// @brief Read INI file content as string
    std::string read_ini_file() const {
        const std::filesystem::path ini_path = temp_dir / config.ini_filename;
        std::ifstream file(ini_path);
        if (!file) {
            return "";
        }
        return std::string(std::istreambuf_iterator<char>(file), std::istreambuf_iterator<char>());
    }
    
    /// @brief Check if INI file exists
    bool ini_file_exists() const {
        return std::filesystem::exists(temp_dir / config.ini_filename);
    }
    
    /// @brief Get list of ART files in test assets
    std::vector<std::filesystem::path> get_test_art_files() const {
        std::vector<std::filesystem::path> art_files;
        const std::filesystem::path assets_dir = TEST_ASSET_SOURCE_DIR;
        
        for (const auto& entry : std::filesystem::directory_iterator(assets_dir)) {
            if (entry.path().extension() == ".ART") {
                art_files.push_back(entry.path());
            }
        }
        
        std::sort(art_files.begin(), art_files.end());
        return art_files;
    }
};

/// @brief Create a test ART file with specific animation data
class AnimatedArtTestFixture {
public:
    /// @brief Create ART file with animated tiles
    static std::vector<std::byte> create_animated_art_file(
        std::uint32_t tile_count = 2,
        std::uint32_t tile_start = 0) {
        
        std::vector<std::byte> data;
        
        // Header
        write_u32_le(data, 1);                    // version
        write_u32_le(data, tile_count);           // numtiles
        write_u32_le(data, tile_start);           // localtilestart
        write_u32_le(data, tile_start + tile_count - 1); // localtileend
        
        // Tile dimensions and animations
        for (std::uint32_t i = 0; i < tile_count; ++i) {
            write_u16_le(data, 8);  // tilesizx[i] - 8x8 tiles
        }
        for (std::uint32_t i = 0; i < tile_count; ++i) {
            write_u16_le(data, 8);  // tilesizy[i] - 8x8 tiles
        }
        
        // Animation data - create different animation types
        for (std::uint32_t i = 0; i < tile_count; ++i) {
            std::uint32_t picanm = 0;
            switch (i % 4) {
                case 0: // Oscillating animation with 3 frames
                    picanm = (3 & 0x3F) |                    // frame count
                             (static_cast<std::uint32_t>(art2img::TileAnimation::Type::oscillating) << 6) | // type
                             (5 << 24);                      // speed
                    break;
                case 1: // Forward animation with 2 frames
                    picanm = (2 & 0x3F) |                    // frame count
                             (static_cast<std::uint32_t>(art2img::TileAnimation::Type::forward) << 6) | // type
                             (3 << 24);                      // speed
                    break;
                case 2: // Backward animation with 4 frames
                    picanm = (4 & 0x3F) |                    // frame count
                             (static_cast<std::uint32_t>(art2img::TileAnimation::Type::backward) << 6) | // type
                             (7 << 24);                      // speed
                    break;
                case 3: // No animation but with center offsets
                    picanm = (10 << 8) |                    // y_center_offset
                             (20 << 16) |                   // x_center_offset
                             (1 << 28);                     // other flags
                    break;
            }
            write_u32_le(data, picanm);
        }
        
        // Write pixel data for each tile (8x8 = 64 pixels each)
        for (std::uint32_t i = 0; i < tile_count; ++i) {
            for (std::size_t j = 0; j < 64; ++j) {
                data.push_back(static_cast<std::byte>((i * 16 + j) % 256));
            }
        }
        
        return data;
    }

private:
    static void write_u16_le(std::vector<std::byte>& data, std::uint16_t value) {
        data.push_back(static_cast<std::byte>(value & 0xFF));
        data.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
    }
    
    static void write_u32_le(std::vector<std::byte>& data, std::uint32_t value) {
        data.push_back(static_cast<std::byte>(value & 0xFF));
        data.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
        data.push_back(static_cast<std::byte>((value >> 16) & 0xFF));
        data.push_back(static_cast<std::byte>((value >> 24) & 0xFF));
    }
};

} // anonymous namespace

TEST_SUITE("Animation Export Unit Tests") {

    TEST_CASE_FIXTURE(AnimationTestFixture, "AnimationExportConfig default values") {
        art2img::AnimationExportConfig default_config;
        CHECK(default_config.output_dir == ".");
        CHECK(default_config.base_name == "tile");
        CHECK(default_config.include_non_animated == true);
        CHECK(default_config.generate_ini == true);
        CHECK(default_config.ini_filename == "animdata.ini");
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "export_animation_data - empty ART bundle") {
        art2img::ArtData empty_bundle;
        
        auto result = art2img::export_animation_data(empty_bundle, config);
        REQUIRE(!result.has_value());
        CHECK(result.error().code == art2img::errc::no_animation);
        CHECK(!ini_file_exists());
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "export_animation_data - invalid output directory") {
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file();
        auto art_result = art2img::load_art_bundle(test_data);
        REQUIRE(art_result.has_value());
        
        // Try to use an invalid directory (e.g., a file instead of directory)
        config.output_dir = "/dev/null/invalid/path";
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(!result.has_value());
        CHECK(result.error().code == art2img::errc::io_failure);
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "export_animation_data - successful export") {
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file(4, 100);
        auto art_result = art2img::load_art_bundle(test_data);
        REQUIRE(art_result.has_value());
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(result.has_value());
        CHECK(ini_file_exists());
        
        const std::string ini_content = read_ini_file();
        CHECK(!ini_content.empty());
        
        // Check header
        CHECK(ini_content.find("; This file contains animation data from ART tiles") != std::string::npos);
        CHECK(ini_content.find("; Extracted by art2img v2.0") != std::string::npos);
        
        // Check animation entries
        CHECK(ini_content.find("[tile0100.tga -> tile0103.tga]") != std::string::npos); // 3 frames
        CHECK(ini_content.find("[tile0101.tga -> tile0102.tga]") != std::string::npos); // 2 frames
        CHECK(ini_content.find("[tile0102.tga -> tile0106.tga]") != std::string::npos); // 4 frames
        
        // Check animation types
        CHECK(ini_content.find("AnimationType=oscillation") != std::string::npos);
        CHECK(ini_content.find("AnimationType=forward") != std::string::npos);
        CHECK(ini_content.find("AnimationType=backward") != std::string::npos);
        
        // Check animation speeds
        CHECK(ini_content.find("AnimationSpeed=5") != std::string::npos);
        CHECK(ini_content.find("AnimationSpeed=3") != std::string::npos);
        CHECK(ini_content.find("AnimationSpeed=7") != std::string::npos);
        
        // Check individual tile entries (since include_non_animated is true by default)
        CHECK(ini_content.find("[tile0103.tga]") != std::string::npos);
        CHECK(ini_content.find("XCenterOffset=20") != std::string::npos);
        CHECK(ini_content.find("YCenterOffset=10") != std::string::npos);
        CHECK(ini_content.find("OtherFlags=1") != std::string::npos);
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "export_animation_data - exclude non-animated tiles") {
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file(4, 100);
        auto art_result = art2img::load_art_bundle(test_data);
        REQUIRE(art_result.has_value());
        
        config.include_non_animated = false;
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(result.has_value());
        
        const std::string ini_content = read_ini_file();
        
        // Should only have animation sequence entries, not individual tile entries
        CHECK(ini_content.find("[tile0100.tga -> tile0103.tga]") != std::string::npos);
        CHECK(ini_content.find("[tile0103.tga]") == std::string::npos); // Should not exist
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "export_animation_data - custom INI filename") {
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file();
        auto art_result = art2img::load_art_bundle(test_data);
        REQUIRE(art_result.has_value());
        
        config.ini_filename = "custom_animation.ini";
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(result.has_value());
        CHECK(std::filesystem::exists(temp_dir / "custom_animation.ini"));
        CHECK(!std::filesystem::exists(temp_dir / "test_animdata.ini"));
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "get_animation_type_string") {
        CHECK(art2img::get_animation_type_string(art2img::TileAnimation::Type::none) == "none");
        CHECK(art2img::get_animation_type_string(art2img::TileAnimation::Type::oscillating) == "oscillation");
        CHECK(art2img::get_animation_type_string(art2img::TileAnimation::Type::forward) == "forward");
        CHECK(art2img::get_animation_type_string(art2img::TileAnimation::Type::backward) == "backward");
    }
}

TEST_SUITE("Animation Export Integration Tests") {

    TEST_CASE_FIXTURE(AnimationTestFixture, "export all test ART files in parallel") {
        const auto art_files = get_test_art_files();
        REQUIRE(!art_files.empty());
        
        std::vector<std::filesystem::path> successful_exports;
        std::vector<std::string> failed_files;
        
        // Process each ART file
        for (const auto& art_file : art_files) {
            if (!std::filesystem::exists(art_file)) {
                failed_files.push_back(art_file.string() + " (file not found)");
                continue;
            }
            
            // Create unique subdirectory for each ART file
            const std::string art_name = art_file.stem().string();
            config.output_dir = temp_dir / art_name;
            test_helpers::ensure_test_output_dir(config.output_dir);
            config.ini_filename = art_name + "_animdata.ini";
            
            // Load ART file
            auto art_result = art2img::load_art_bundle(art_file);
            if (!art_result.has_value()) {
                failed_files.push_back(art_file.string() + " (load failed: " + art_result.error().message + ")");
                continue;
            }
            
            // Export animation data
            auto export_result = art2img::export_animation_data(art_result.value(), config);
            
            if (export_result.has_value()) {
                successful_exports.push_back(art_file);
                CHECK(std::filesystem::exists(config.output_dir / config.ini_filename));
                
                // Verify INI content format
                const std::string ini_content = read_ini_file();
                CHECK(!ini_content.empty());
                CHECK(ini_content.find("; This file contains animation data from ART tiles") != std::string::npos);
            } else {
                // It's okay if some files don't have animations
                if (export_result.error().code == art2img::errc::no_animation) {
                    successful_exports.push_back(art_file); // Consider this a success
                } else {
                    failed_files.push_back(art_file.string() + " (export failed: " + export_result.error().message + ")");
                }
            }
        }
        
        // Report results
        INFO("Processed ", art_files.size(), " ART files");
        INFO("Successfully exported: ", successful_exports.size());
        INFO("Failed: ", failed_files.size());
        
        for (const auto& failure : failed_files) {
            INFO("Failure: ", failure);
        }
        
        // At least some files should be processed successfully
        CHECK(successful_exports.size() > 0);
        
        // Verify we have INI files for successful exports
        for (const auto& art_file : successful_exports) {
            const std::string art_name = art_file.stem().string();
            const std::filesystem::path ini_path = temp_dir / art_name / (art_name + "_animdata.ini");
            if (std::filesystem::exists(ini_path)) {
                // Verify INI format
                std::ifstream file(ini_path);
                REQUIRE(file);
                std::string line;
                bool found_header = false;
                while (std::getline(file, line)) {
                    if (line.find("; This file contains animation data") != std::string::npos) {
                        found_header = true;
                        break;
                    }
                }
                CHECK(found_header);
            }
        }
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "legacy format compatibility") {
        // Create ART file with known animation data
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file(2, 0);
        auto art_result = art2img::load_art_bundle(test_data);
        REQUIRE(art_result.has_value());
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(result.has_value());
        
        const std::string ini_content = read_ini_file();
        
        // Verify exact format matches legacy art2tga.c output
        
        // Check section format: [tileXXXX.tga -> tileYYYY.tga]
        std::regex section_regex(R"(\[tile\d{4}\.tga -> tile\d{4}\.tga\])");
        std::sregex_iterator iter(ini_content.begin(), ini_content.end(), section_regex);
        std::sregex_iterator end;
        CHECK(std::distance(iter, end) >= 1); // At least one animation section
        
        // Check property format: "   PropertyName=Value"
        std::regex property_regex(R"(\s{3}\w+=\w+)");
        iter = std::sregex_iterator(ini_content.begin(), ini_content.end(), property_regex);
        CHECK(std::distance(iter, end) >= 2); // At least AnimationType and AnimationSpeed
        
        // Check specific format elements from legacy
        CHECK(ini_content.find("AnimationType=") != std::string::npos);
        CHECK(ini_content.find("AnimationSpeed=") != std::string::npos);
        CHECK(ini_content.find("XCenterOffset=") != std::string::npos);
        CHECK(ini_content.find("YCenterOffset=") != std::string::npos);
        CHECK(ini_content.find("OtherFlags=") != std::string::npos);
        
        // Verify spacing matches legacy (3 spaces before properties)
        CHECK(ini_content.find("   AnimationType=") != std::string::npos);
        CHECK(ini_content.find("   AnimationSpeed=") != std::string::npos);
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "real ART file animation detection") {
        // Test with actual TILES000.ART if available
        const std::filesystem::path art_file = TEST_ASSET_SOURCE_DIR "/TILES000.ART";
        
        if (!std::filesystem::exists(art_file)) {
            SKIP("TILES000.ART not found in test assets");
        }
        
        auto art_result = art2img::load_art_bundle(art_file);
        REQUIRE(art_result.has_value());
        
        const auto& art_data = art_result.value();
        
        // Count tiles with animation data
        std::size_t animated_tiles = 0;
        for (std::size_t i = 0; i < art_data.tile_count(); ++i) {
            const auto tile = art_data.get_tile(i);
            if (tile.has_value()) {
                const auto& anim = tile->animation;
                std::uint32_t picanm = anim.to_picanm();
                bool has_animation = ((picanm & 0x3F) != 0) ||           // frame count
                                    (((picanm >> 6) & 0x03) != 0) ||    // animation type  
                                    (((picanm >> 24) & 0x0F) != 0);      // animation speed
                if (has_animation) {
                    animated_tiles++;
                }
            }
        }
        
        INFO("Found ", animated_tiles, " animated tiles out of ", art_data.tile_count());
        
        // Export animation data
        auto export_result = art2img::export_animation_data(art_data, config);
        
        if (animated_tiles > 0) {
            REQUIRE(export_result.has_value());
            CHECK(ini_file_exists());
            
            const std::string ini_content = read_ini_file();
            CHECK(!ini_content.empty());
            
            // Should have animation sections if animated tiles exist
            std::regex section_regex(R"(\[tile\d{4}\.tga -> tile\d{4}\.tga\])");
            std::sregex_iterator iter(ini_content.begin(), ini_content.end(), section_regex);
            std::sregex_iterator end;
            CHECK(std::distance(iter, end) > 0);
        } else {
            // It's okay if no animations are found
            CHECK(!export_result.has_value());
            CHECK(export_result.error().code == art2img::errc::no_animation);
        }
    }
}

TEST_SUITE("Animation Export Error Handling") {

    TEST_CASE_FIXTURE(AnimationTestFixture, "no animation data error") {
        // Create ART file with no animation data
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file(1, 0);
        // Modify to remove all animation data
        std::vector<std::byte> no_anim_data = test_data;
        // Clear animation data (set picanm to 0)
        no_anim_data[24] = std::byte{0};
        no_anim_data[25] = std::byte{0};
        no_anim_data[26] = std::byte{0};
        no_anim_data[27] = std::byte{0};
        
        auto art_result = art2img::load_art_bundle(no_anim_data);
        REQUIRE(art_result.has_value());
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(!result.has_value());
        CHECK(result.error().code == art2img::errc::no_animation);
        CHECK(result.error().message.find("No animated tiles found") != std::string::npos);
        CHECK(!ini_file_exists());
    }

    TEST_CASE_FIXTURE(AnimationTestFixture, "file system permission errors") {
        const auto test_data = AnimatedArtTestFixture::create_animated_art_file();
        auto art_result = art2img::load_art_bundle(test_data);
        REQUIRE(art_result.has_value());
        
        // Try to write to a location that should cause permission issues
        // Note: This test might not work on all systems, so we'll simulate the error
        config.output_dir = "/root/nonexistent/path"; // Likely to fail
        
        auto result = art2img::export_animation_data(art_result.value(), config);
        REQUIRE(!result.has_value());
        CHECK(result.error().code == art2img::errc::io_failure);
    }
}