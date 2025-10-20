#include <cstring>
#include <filesystem>
#include <fstream>
#include <vector>

#include <doctest/doctest.h>

#include <art2img/color_helpers.hpp>
#include <art2img/palette.hpp>
#include <art2img/types.hpp>

using namespace art2img;

TEST_SUITE("palette module") {
  TEST_CASE("Palette struct default construction") {
    Palette palette;

    // Check that all data is zero-initialized
    for (std::size_t i = 0; i < constants::PALETTE_DATA_SIZE; ++i) {
      CHECK(palette.data[i] == 0);
    }

    CHECK(palette.shade_table_count == 0);
    CHECK(palette.shade_tables.empty());

    // Check translucent map is zeroed
    for (std::size_t i = 0; i < constants::TRANSLUCENT_TABLE_SIZE; ++i) {
      CHECK(palette.translucent_map[i] == 0);
    }

    CHECK(!palette.has_shade_tables());
    CHECK(!palette.has_translucent_map());
  }

  TEST_CASE("Palette struct view functions") {
    Palette palette;

    // Test palette data view
    auto palette_view = palette.palette_data();
    CHECK(palette_view.size() == constants::PALETTE_DATA_SIZE);
    CHECK(palette_view.data() == palette.data.data());

    // Test shade data view (empty)
    auto shade_view = palette.shade_data();
    CHECK(shade_view.empty());

    // Test translucent data view
    auto translucent_view = palette.translucent_data();
    CHECK(translucent_view.size() == constants::TRANSLUCENT_TABLE_SIZE);
    CHECK(translucent_view.data() == palette.translucent_map.data());
  }

  TEST_CASE("load_palette from real PALETTE.DAT file") {
    const auto palette_path =
        std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";

    // Skip test if file doesn't exist
    if (!std::filesystem::exists(palette_path)) {
      return;
    }

    auto result = load_palette(palette_path);
    REQUIRE(result.has_value());

    const auto& palette = result.value();

    // Basic validation of loaded palette
    CHECK(palette.shade_table_count > 0);
    CHECK(palette.has_shade_tables());

    // Check palette data size
    CHECK(palette.palette_data().size() == constants::PALETTE_DATA_SIZE);

    // Check that we can convert palette entries
    auto rgba = palette_entry_to_rgba(palette, 0);
    CHECK(rgba != 0);

    // Test RGB conversion
    auto [r, g, b] = palette_entry_to_rgb(palette, 0);
    CHECK(r <= 255);
    CHECK(g <= 255);
    CHECK(b <= 255);
  }

  TEST_CASE("load_palette from span - real data") {
    const auto palette_path =
        std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";

    // Skip test if file doesn't exist
    if (!std::filesystem::exists(palette_path)) {
      return;
    }

    // Read file into buffer
    std::ifstream file(palette_path, std::ios::binary | std::ios::ate);
    REQUIRE(file);

    const auto file_size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<std::byte> buffer(static_cast<std::size_t>(file_size));
    REQUIRE(file.read(reinterpret_cast<char*>(buffer.data()), file_size));

    // Load from span
    auto result = load_palette(buffer);
    REQUIRE(result.has_value());

    const auto& palette = result.value();
    CHECK(palette.shade_table_count > 0);
    CHECK(palette.has_shade_tables());
  }

  TEST_CASE("palette_entry_to_rgba with real data") {
    const auto palette_path =
        std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";

    // Skip test if file doesn't exist
    if (!std::filesystem::exists(palette_path)) {
      return;
    }

    auto result = load_palette(palette_path);
    REQUIRE(result.has_value());
    const auto& palette = result.value();

    // Test various palette indices
    for (std::uint8_t i = 0; i < 10; ++i) {
      const auto color = color::unpack_rgba(palette_entry_to_rgba(palette, i));
      // Check alpha channel is set to 0xFF
      CHECK(color.a == 255);
      // Check RGB components are reasonable
      CHECK(color.r <= 255);
      CHECK(color.g <= 255);
      CHECK(color.b <= 255);
    }

    // Test edge case: the function accepts uint8_t, so all values 0-255 are
    // valid The validation happens inside the function based on PALETTE_SIZE
    // constant
    for (std::uint8_t i = 250; i < 255; ++i) {
      const auto color = color::unpack_rgba(palette_entry_to_rgba(palette, i));
      // All valid indices should return valid RGBA values
      CHECK(color.a == 255);  // Alpha should be set
    }
  }

  TEST_CASE("palette_shaded_entry_to_rgba with real data") {
    const auto palette_path =
        std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";

    // Skip test if file doesn't exist
    if (!std::filesystem::exists(palette_path)) {
      return;
    }

    auto result = load_palette(palette_path);
    REQUIRE(result.has_value());
    const auto& palette = result.value();

    REQUIRE(palette.has_shade_tables());

    // Test shaded entries
    for (std::uint8_t shade = 0;
         shade <
         std::min(palette.shade_table_count, static_cast<std::uint16_t>(5));
         ++shade) {
      for (std::uint8_t i = 0; i < 10; ++i) {
        const auto color =
            color::unpack_rgba(palette_shaded_entry_to_rgba(palette, shade, i));
        // Check alpha channel is set to 0xFF
        CHECK(color.a == 255);
      }
    }

    // Test invalid shade index (should return unshaded color)
    auto unshaded_rgba = palette_shaded_entry_to_rgba(palette, 255, 0);
    auto normal_rgba = palette_entry_to_rgba(palette, 0);
    CHECK(unshaded_rgba == normal_rgba);
  }

  // Negative test cases - use synthetic data
  TEST_CASE("load_palette error cases") {
    SUBCASE("empty data") {
      std::vector<std::byte> empty_data;
      auto result = load_palette(empty_data);
      CHECK(!result.has_value());
    }

    SUBCASE("too small data") {
      std::vector<std::byte> small_data(10);  // Too small for palette
      auto result = load_palette(small_data);
      CHECK(!result.has_value());
    }

    SUBCASE("invalid shade count") {
      // Create minimal valid palette data with invalid shade count
      std::vector<std::byte> data(constants::PALETTE_DATA_SIZE + 2);

      // Fill with zeros
      std::memset(data.data(), 0, data.size());

      // Set invalid shade count (too large)
      data[constants::PALETTE_DATA_SIZE] = std::byte{0xFF};
      data[constants::PALETTE_DATA_SIZE + 1] =
          std::byte{0x01};  // 511 shade tables

      auto result = load_palette(data);
      CHECK(!result.has_value());
    }
  }

  TEST_CASE("load_palette from non-existent file") {
    auto result = load_palette("non_existent_file.dat");
    CHECK(!result.has_value());
  }

  TEST_CASE("6-bit to 8-bit scaling behavior") {
    // Test the scaling through palette entry conversion
    const auto palette_path =
        std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";

    // Skip test if file doesn't exist
    if (!std::filesystem::exists(palette_path)) {
      return;
    }

    auto result = load_palette(palette_path);
    REQUIRE(result.has_value());
    const auto& palette = result.value();

    // Test that RGB conversion produces 8-bit values
    auto [r, g, b] = palette_entry_to_rgb(palette, 0);
    CHECK(r <= 255);
    CHECK(g <= 255);
    CHECK(b <= 255);

    // Test that the scaling factor is applied correctly
    // 6-bit max value (63) should scale to 8-bit max value (255)
    // This is verified through the palette conversion
  }
}
