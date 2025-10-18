#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "art2img/exceptions.hpp"
#include "art2img/extractor_api.hpp"
#include "art2img/image_writer.hpp"
#include "test_helpers.hpp"

// Test full extraction of all available ART files
TEST_CASE("Full ART file extraction") {
  // Skip test if required assets are not available
  if (!has_test_asset("TILES000.ART") || !has_test_asset("PALETTE.DAT")) {
    MESSAGE("Required assets not found, skipping full extraction tests");
    return;
  }

  // Create unique test directory
  std::string timestamp = std::to_string(std::time(nullptr));
  std::string test_dir = "build/tests/full_extraction_" + timestamp;
  std::filesystem::create_directories(test_dir);

  // List of ART files to test (in order of availability)
  std::vector<std::string> art_files = {"TILES000.ART", "TILES001.ART", "TILES002.ART",
                                        "TILES003.ART"};

  art2img::ImageWriter::Options options;
  options.fix_transparency = true;
  options.enable_alpha = true;

  int total_files_extracted = 0;
  int total_art_files_processed = 0;

  for (const auto& art_filename : art_files) {
    if (!has_test_asset(art_filename)) {
      continue;  // Skip if file not available
    }

    // Create subdirectory for this ART file
    std::string art_output_dir = test_dir + "/" + art_filename.substr(0, art_filename.find('.'));
    std::filesystem::create_directories(art_output_dir);

    // Load ART file
    auto art_data = load_test_asset(art_filename);
    if (art_data.empty()) {
      continue;
    }

    auto palette_data = load_test_asset("PALETTE.DAT");
    if (palette_data.size() < art2img::Palette::SIZE) {
      continue;
    }

    art2img::ExtractorAPI extractor;
    if (!extractor.load_art_from_memory(art_data.data(), art_data.size())) {
      continue;
    }

    if (!extractor.load_palette_from_memory(palette_data.data(), palette_data.size())) {
      continue;
    }

    // Extract all tiles to PNG files
    uint32_t tile_count = extractor.get_tile_count();
    int files_extracted = 0;

    for (uint32_t i = 0; i < tile_count; ++i) {
      auto result = extractor.extract_tile(i, art2img::ImageFormat::PNG, options);
      if (result.success && !result.image_data.empty()) {
        std::string filename =
            art_output_dir + "/tile" + std::to_string(result.tile_index) + ".png";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()),
                   result.image_data.size());
        file.close();

        if (std::filesystem::exists(filename) && std::filesystem::file_size(filename) > 0) {
          files_extracted++;
          total_files_extracted++;
        }
      }
    }

    total_art_files_processed++;

    // Verify this ART file produced output files
    CHECK_GT(files_extracted, 0);

    // Verify output directory has expected files
    int actual_file_count = 0;
    for (const auto& entry : std::filesystem::directory_iterator(art_output_dir)) {
      if (entry.is_regular_file() && entry.path().extension() == ".png") {
        actual_file_count++;
      }
    }
    CHECK_EQ(actual_file_count, files_extracted);
  }

  // Verify we processed at least one ART file
  CHECK_GT(total_art_files_processed, 0);
  CHECK_GT(total_files_extracted, 0);

  // Only cleanup in CI environment
  if (std::getenv("CI") != nullptr) {
    std::filesystem::remove_all(test_dir);
  }
}