#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <filesystem>
#include <fstream>
#include <vector>

#include "art2img/extractor_api.hpp"
#include "test_helpers.hpp"

TEST_CASE("ExtractorAPI - Library API comprehensive tests") {
  art2img::ExtractorAPI extractor;

  // Load ART file from memory
  const auto art_path = test_asset_path("TILES000.ART");
  std::ifstream art_file(art_path, std::ios::binary);
  REQUIRE(art_file.is_open());

  // Read file into memory
  art_file.seekg(0, std::ios::end);
  size_t art_size = art_file.tellg();
  art_file.seekg(0, std::ios::beg);

  std::vector<uint8_t> art_data(art_size);
  art_file.read(reinterpret_cast<char*>(art_data.data()), art_size);
  art_file.close();

  // Load ART data from memory
  bool art_load_result = extractor.load_art_from_memory(art_data.data(), art_size);
  CHECK(art_load_result);

  if (art_load_result) {
    CHECK_GT(extractor.get_tile_count(), 0);
  }

  // Load palette from memory
  const auto palette_path = test_asset_path("PALETTE.DAT");
  std::ifstream palette_file(palette_path, std::ios::binary);
  REQUIRE(palette_file.is_open());

  // Read palette into memory
  palette_file.seekg(0, std::ios::end);
  size_t palette_size = palette_file.tellg();
  palette_file.seekg(0, std::ios::beg);

  std::vector<uint8_t> palette_data(palette_size);
  palette_file.read(reinterpret_cast<char*>(palette_data.data()), palette_size);
  palette_file.close();

  // Load palette data from memory
  bool palette_load_result = extractor.load_palette_from_memory(palette_data.data(), palette_size);
  CHECK(palette_load_result);

  // Extract a single tile to PNG
  art2img::ExtractionResult result = extractor.extract_tile(0u, art2img::ImageFormat::PNG);
  CHECK(result.success);

  if (result.success) {
    CHECK_GT(result.width, 0);
    CHECK_GT(result.height, 0);
    CHECK_GT(result.image_data.size(), 0);
  }

  // Test batch extraction
  std::vector<art2img::ExtractionResult> results = extractor.extract_all_tiles_png();
  CHECK_GT(results.size(), 0);

  // Count successful extractions
  size_t success_count = 0;
  for (const auto& res : results) {
    if (res.success) {
      success_count++;
    }
  }

  CHECK_GT(success_count, 0);
}
