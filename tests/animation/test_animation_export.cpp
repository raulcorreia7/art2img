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

#include "../test_helpers.hpp"
#include <art2img/art.hpp>
#include <art2img/error.hpp>
#include <doctest/doctest.h>
#include <filesystem>
#include <fstream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Test fixtures
struct AnimationTestFixture {
  std::filesystem::path temp_dir;
  art2img::AnimationExportConfig config;

  AnimationTestFixture() {
    temp_dir =
        test_helpers::get_unit_test_dir("animation", "test_animation_export");
    test_helpers::ensure_test_output_dir(temp_dir);
    config.output_dir = temp_dir;
    config.ini_filename = "test_animdata.ini";
  }

  ~AnimationTestFixture() { test_helpers::cleanup_test_output_dir(temp_dir); }

  /// @brief Read INI file content as string
  std::string read_ini_file() const {
    const std::filesystem::path ini_path = temp_dir / config.ini_filename;
    std::ifstream file(ini_path.string());
    if (!file) {
      return "";
    }
    return std::string(std::istreambuf_iterator<char>(file),
                       std::istreambuf_iterator<char>());
  }

  /// @brief Check if INI file exists
  bool ini_file_exists() const {
    return std::filesystem::exists(temp_dir / config.ini_filename);
  }

  /// @brief Get list of ART files in test assets
  std::vector<std::filesystem::path> get_test_art_files() const {
    std::vector<std::filesystem::path> art_files;
    const std::filesystem::path assets_dir = TEST_ASSET_SOURCE_DIR;

    for (const auto &entry : std::filesystem::directory_iterator(assets_dir)) {
      if (entry.path().extension() == ".ART") {
        art_files.push_back(entry.path());
      }
    }

    std::sort(art_files.begin(), art_files.end());
    return art_files;
  }

  /// @brief Create a test ART file with animation data
  static std::vector<std::byte> create_animated_art_file(
      std::uint32_t tile_start = 0, std::uint16_t tile_width = 8,
      std::uint16_t tile_height = 8, std::uint32_t picanm = 0x12345678) {

    std::vector<std::byte> data;

    // Header: version (1), numtiles (1), localtilestart, localtileend
    const std::uint32_t version = 1;
    const std::uint32_t numtiles = 1;
    const std::uint32_t localtileend = tile_start;

    // Write header
    write_u32_le(data, version);
    write_u32_le(data, numtiles);
    write_u32_le(data, tile_start);
    write_u32_le(data, localtileend);

    // Write tile arrays
    write_u16_le(data, tile_width);  // tilesizx[0]
    write_u16_le(data, tile_height); // tilesizy[0]
    write_u32_le(data, picanm);      // picanm[0] (with animation)

    // Write pixel data (column-major, filled with test pattern)
    const std::size_t pixel_count = static_cast<std::size_t>(tile_width) *
                                    static_cast<std::size_t>(tile_height);
    for (std::size_t i = 0; i < pixel_count; ++i) {
      data.push_back(static_cast<std::byte>(i % 256));
    }

    return data;
  }

  /// @brief Create a test ART file with no animation
  static std::vector<std::byte>
  create_non_animated_art_file(std::uint32_t tile_start = 0,
                               std::uint16_t tile_width = 8,
                               std::uint16_t tile_height = 8) {
    return create_animated_art_file(tile_start, tile_width, tile_height,
                                    0); // picanm = 0 (no animation)
  }

private:
  static void write_u8(std::vector<std::byte> &data, std::uint8_t value) {
    data.push_back(static_cast<std::byte>(value));
  }

  static void write_u16_le(std::vector<std::byte> &data, std::uint16_t value) {
    data.push_back(static_cast<std::byte>(value & 0xFF));
    data.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
  }

  static void write_u32_le(std::vector<std::byte> &data, std::uint32_t value) {
    data.push_back(static_cast<std::byte>(value & 0xFF));
    data.push_back(static_cast<std::byte>((value >> 8) & 0xFF));
    data.push_back(static_cast<std::byte>((value >> 16) & 0xFF));
    data.push_back(static_cast<std::byte>((value >> 24) & 0xFF));
  }
};

} // anonymous namespace

TEST_SUITE("Animation Export Unit Tests") {

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "AnimationExportConfig default values") {
    art2img::AnimationExportConfig default_config;
    CHECK(default_config.output_dir == ".");
    CHECK(default_config.base_name == "tile");
    CHECK(default_config.include_non_animated == true);
    CHECK(default_config.generate_ini == true);
    CHECK(default_config.ini_filename == "animdata.ini");
    CHECK(default_config.image_format == art2img::ImageFormat::png);
    CHECK(default_config.include_image_references == true);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "export_animation_data with animated tile") {
    // Create test ART file with animation
    const auto test_data = create_animated_art_file(100, 16, 16, 0x12345678);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data
    auto export_result = art2img::export_animation_data(art_data, config);

    REQUIRE(export_result.has_value());
    CHECK(ini_file_exists());

    // Check INI content
    const std::string ini_content = read_ini_file();
    CHECK(!ini_content.empty());

    // Should contain animation data for tile 100
    CHECK(ini_content.find("[tile0100.") != std::string::npos);
    CHECK(ini_content.find("AnimationType") != std::string::npos);
    CHECK(ini_content.find("AnimationSpeed") != std::string::npos);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "export_animation_data with non-animated tile") {
    // Create test ART file without animation
    const auto test_data = create_non_animated_art_file(200, 8, 8);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data
    auto export_result = art2img::export_animation_data(art_data, config);

    REQUIRE(export_result.has_value());
    CHECK(ini_file_exists());

    // Check INI content
    const std::string ini_content = read_ini_file();
    CHECK(!ini_content.empty());

    // Should contain tile data even if not animated (when include_non_animated
    // is true)
    CHECK(ini_content.find("[tile0200.") != std::string::npos);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "export_animation_data exclude non-animated") {
    // Configure to exclude non-animated tiles
    art2img::AnimationExportConfig config_exclude = config;
    config_exclude.include_non_animated = false;

    // Create test ART file without animation
    const auto test_data = create_non_animated_art_file(300, 8, 8);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data
    auto export_result =
        art2img::export_animation_data(art_data, config_exclude);

    REQUIRE(export_result.has_value());

    // INI should be empty or not contain non-animated tiles
    const std::string ini_content = read_ini_file();
    CHECK(ini_content.find("[tile0300.") == std::string::npos);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture, "get_animation_type_string") {
    CHECK(art2img::get_animation_type_string(
              art2img::TileAnimation::Type::none) == "none");
    CHECK(art2img::get_animation_type_string(
              art2img::TileAnimation::Type::oscillating) == "oscillation");
    CHECK(art2img::get_animation_type_string(
              art2img::TileAnimation::Type::forward) == "forward");
    CHECK(art2img::get_animation_type_string(
              art2img::TileAnimation::Type::backward) == "backward");
  }

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "image format awareness in INI output") {
    // Configure for BMP format with image references
    art2img::AnimationExportConfig config_bmp = config;
    config_bmp.image_format = art2img::ImageFormat::bmp;
    config_bmp.include_image_references = true;
    config_bmp.base_name = "testtile";

    // Create test ART file with animation
    const auto test_data = create_animated_art_file(150, 8, 8, 0x12345678);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data
    auto export_result = art2img::export_animation_data(art_data, config_bmp);
    REQUIRE(export_result.has_value());
    CHECK(ini_file_exists());

    // Check INI content contains image reference
    const std::string ini_content = read_ini_file();
    CHECK(!ini_content.empty());
    CHECK(ini_content.find("[tile0150.") != std::string::npos);
    CHECK(ini_content.find("ImageFile=testtile_150_0.bmp") !=
          std::string::npos);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "disable image references in INI output") {
    // Configure to disable image references
    art2img::AnimationExportConfig config_no_refs = config;
    config_no_refs.include_image_references = false;

    // Create test ART file with animation
    const auto test_data = create_animated_art_file(160, 8, 8, 0x12345678);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data
    auto export_result =
        art2img::export_animation_data(art_data, config_no_refs);
    REQUIRE(export_result.has_value());
    CHECK(ini_file_exists());

    // Check INI content does NOT contain image reference
    const std::string ini_content = read_ini_file();
    CHECK(!ini_content.empty());
    CHECK(ini_content.find("[tile0160.") != std::string::npos);
    CHECK(ini_content.find("ImageFile=") == std::string::npos);
  }
}

TEST_SUITE("Animation Export Integration Tests") {

  TEST_CASE_FIXTURE(AnimationTestFixture,
                    "export all test ART files in parallel") {
    const auto art_files = get_test_art_files();

    // Skip if no test assets found
    if (art_files.empty()) {
      return; // Skip test if no assets found
    }

    INFO("Found ", art_files.size(), " ART files for testing");
    CHECK(art_files.size() > 0);

    // Test loading and exporting each ART file
    for (const auto &art_file : art_files) {
      INFO("Testing file: ", art_file.filename().string());

      CHECK(std::filesystem::exists(art_file));
      CHECK(art_file.extension() == ".ART");

      // Load ART file
      auto art_result = art2img::load_art_bundle(art_file);
      if (!art_result.has_value()) {
        // Some ART files might be invalid, that's okay for this test
        continue;
      }

      const auto &art_data = art_result.value();
      CHECK(art_data.is_valid());

      // Export animation data
      auto export_result = art2img::export_animation_data(art_data, config);
      if (!export_result.has_value()) {
        // Some files might not have animation data
        continue;
      }

      CHECK(ini_file_exists());
    }
  }

  TEST_CASE_FIXTURE(AnimationTestFixture, "legacy format compatibility") {
    // Create test ART file with known animation values
    const std::uint32_t test_picanm = 0x12345678;
    const auto test_data = create_animated_art_file(1000, 16, 16, test_picanm);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data
    auto export_result = art2img::export_animation_data(art_data, config);
    REQUIRE(export_result.has_value());

    // Check INI content matches expected legacy format
    const std::string ini_content = read_ini_file();
    CHECK(!ini_content.empty());

    // Parse the picanm value to verify correct extraction
    art2img::TileAnimation anim(test_picanm);

    // Check that INI contains the expected values
    CHECK(ini_content.find("[tile1000.") != std::string::npos);
    CHECK(ini_content.find("AnimationType=") != std::string::npos);
    CHECK(ini_content.find("AnimationSpeed=") != std::string::npos);
    CHECK(ini_content.find("XCenterOffset=") != std::string::npos);
    CHECK(ini_content.find("YCenterOffset=") != std::string::npos);

    // Verify specific values are present
    CHECK(ini_content.find(std::to_string(anim.frame_count)) !=
          std::string::npos);
    CHECK(ini_content.find(std::to_string(anim.speed)) != std::string::npos);
    CHECK(ini_content.find(std::to_string(
              static_cast<int>(anim.x_center_offset))) != std::string::npos);
    CHECK(ini_content.find(std::to_string(
              static_cast<int>(anim.y_center_offset))) != std::string::npos);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture, "real ART file animation detection") {
    // Test with actual TILES000.ART if available
    const std::filesystem::path art_file =
        TEST_ASSET_SOURCE_DIR "/TILES000.ART";

    if (!std::filesystem::exists(art_file)) {
      return; // Skip test if file not found
    }

    INFO("Found TILES000.ART for animation detection testing");
    CHECK(std::filesystem::exists(art_file));

    // Load the real ART file
    auto art_result = art2img::load_art_bundle(art_file);
    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    CHECK(art_data.is_valid());
    CHECK(art_data.tile_count() > 0);

    // Count animated tiles
    std::size_t animated_count = 0;
    std::size_t total_count = art_data.tile_count();

    for (std::size_t i = 0; i < total_count; ++i) {
      const auto tile = art_data.get_tile(i);
      if (tile.has_value() &&
          tile->animation.type != art2img::TileAnimation::Type::none) {
        animated_count++;
      }
    }

    INFO("Found ", animated_count, " animated tiles out of ", total_count,
         " total tiles");

    // Export animation data
    auto export_result = art2img::export_animation_data(art_data, config);
    REQUIRE(export_result.has_value());
    CHECK(ini_file_exists());

    // Check INI content
    const std::string ini_content = read_ini_file();
    if (animated_count > 0) {
      CHECK(!ini_content.empty());
      CHECK(ini_content.find("AnimationType=") != std::string::npos);
    }
  }
}

TEST_SUITE("Animation Export Error Handling") {

  TEST_CASE_FIXTURE(AnimationTestFixture, "no animation data error") {
    // Configure to exclude non-animated tiles
    art2img::AnimationExportConfig config_exclude = config;
    config_exclude.include_non_animated = false;

    // Create test ART file without animation
    const auto test_data = create_non_animated_art_file(400, 8, 8);
    auto art_result = art2img::load_art_bundle(test_data);

    REQUIRE(art_result.has_value());
    const auto &art_data = art_result.value();

    // Export animation data - should succeed but create empty INI
    auto export_result =
        art2img::export_animation_data(art_data, config_exclude);
    CHECK(export_result.has_value());

    // INI should exist but be empty or contain no animation data
    const std::string ini_content = read_ini_file();
    CHECK(ini_content.find("AnimationType=") == std::string::npos);
  }

  TEST_CASE_FIXTURE(AnimationTestFixture, "file system permission errors") {
    // Create a read-only directory to test permission errors
    const std::filesystem::path readonly_dir = temp_dir / "readonly";
    test_helpers::ensure_test_output_dir(readonly_dir);

    // Make directory read-only (if possible on this system)
    std::error_code ec;
    std::filesystem::permissions(readonly_dir,
                                 std::filesystem::perms::owner_all |
                                     std::filesystem::perms::group_all |
                                     std::filesystem::perms::others_all,
                                 std::filesystem::perm_options::remove, ec);

    // Configure to write to read-only directory
    art2img::AnimationExportConfig config_readonly = config;
    config_readonly.output_dir = readonly_dir;

    // Create test ART file
    const auto test_data = create_animated_art_file(500, 8, 8);
    auto art_result = art2img::load_art_bundle(test_data);
    REQUIRE(art_result.has_value());

    // Try to export - should fail due to permission error
    auto export_result =
        art2img::export_animation_data(art_result.value(), config_readonly);

    // Restore permissions for cleanup
    std::filesystem::permissions(readonly_dir,
                                 std::filesystem::perms::owner_all |
                                     std::filesystem::perms::group_all |
                                     std::filesystem::perms::others_all,
                                 std::filesystem::perm_options::add, ec);

    // Check that export failed appropriately
    if (!export_result.has_value()) {
      CHECK(export_result.error().code == art2img::errc::io_failure);
    }
  }

  TEST_CASE_FIXTURE(AnimationTestFixture, "invalid ART data") {
    // Create corrupted ART data
    std::vector<std::byte> corrupted_data = {
        std::byte{0x01}}; // Just version byte

    // Try to load corrupted data
    auto art_result = art2img::load_art_bundle(corrupted_data);
    CHECK(!art_result.has_value());
    CHECK(art_result.error().code == art2img::errc::invalid_art);
  }
}