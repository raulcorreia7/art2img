#include <doctest/doctest.h>

#include <art2img/adapters/io.hpp>
#include <art2img/core/palette.hpp>
#include <filesystem>

TEST_SUITE("palette module")
{
  TEST_CASE("Basic palette functionality compilation test")
  {
    // This test verifies that the palette module can be included and basic
    // types exist
    using art2img::core::Palette;
    using art2img::core::PaletteView;

    // Test constants
    CHECK(art2img::core::palette_color_count == 256);
    CHECK(art2img::core::palette_component_count == 768);

    // Basic compilation test - more comprehensive tests will be added
    // as the implementation progresses
    CHECK(true);
  }

  TEST_CASE("Load palette from file")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    // Verify test file exists
    REQUIRE(std::filesystem::exists(palette_file));

    // Load the palette file
    auto file_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(file_data.has_value());
    CHECK(!file_data->empty());

    // Parse the palette
    auto palette = art2img::core::load_palette(*file_data);
    REQUIRE(palette.has_value());

    // Verify palette structure
    CHECK(palette->rgb.size() == art2img::core::palette_component_count);

    // Check that RGB values are in valid range (0-255)
    for (std::size_t i = 0; i < palette->rgb.size(); ++i) {
      CHECK(palette->rgb[i] <= 255);
    }
  }

  TEST_CASE("Create palette view")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    auto file_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(file_data.has_value());

    auto palette = art2img::core::load_palette(*file_data);
    REQUIRE(palette.has_value());

    // Create palette view
    auto view = art2img::core::view_palette(*palette);

    // Verify view structure
    CHECK(view.rgb.size() == art2img::core::palette_component_count);
    CHECK(view.rgb.data() == palette->rgb.data());

    // Test shade table detection
    if (palette->shade_table_count > 0) {
      CHECK(view.has_shades());
      CHECK(view.shade_tables.size() >=
            static_cast<std::size_t>(palette->shade_table_count) *
                art2img::core::shade_table_size);
    }
    else {
      CHECK(!view.has_shades());
    }

    // Check translucent table size
    CHECK(view.translucent.size() == art2img::core::translucent_table_size);
  }

  TEST_CASE("Palette color mapping")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    auto file_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(file_data.has_value());

    auto palette = art2img::core::load_palette(*file_data);
    REQUIRE(palette.has_value());

    auto view = art2img::core::view_palette(*palette);

    // Test accessing specific colors
    REQUIRE(view.rgb.size() >= 3);  // Need at least one color (3 components)

    // Check first color (typically black in many palettes)
    std::uint8_t r0 = view.rgb[0];
    std::uint8_t g0 = view.rgb[1];
    std::uint8_t b0 = view.rgb[2];
    CHECK(r0 <= 255);
    CHECK(g0 <= 255);
    CHECK(b0 <= 255);

    // Check last color if we have full palette
    if (view.rgb.size() >= art2img::core::palette_component_count) {
      std::size_t last_index = art2img::core::palette_color_count - 1;
      std::size_t last_offset = last_index * 3;

      std::uint8_t r_last = view.rgb[last_offset];
      std::uint8_t g_last = view.rgb[last_offset + 1];
      std::uint8_t b_last = view.rgb[last_offset + 2];

      CHECK(r_last <= 255);
      CHECK(g_last <= 255);
      CHECK(b_last <= 255);
    }
  }

  TEST_CASE("Load invalid palette file")
  {
    // Test with empty data
    auto empty_palette =
        art2img::core::load_palette(std::span<const std::byte>{});
    CHECK(!empty_palette.has_value());

    // Test with insufficient data
    std::vector<std::byte> small_data(10, std::byte{0});
    auto small_palette = art2img::core::load_palette(small_data);
    CHECK(!small_palette.has_value());

    // Test with data that's too small for full palette
    std::vector<std::byte> partial_data(100, std::byte{0});
    auto partial_palette = art2img::core::load_palette(partial_data);
    CHECK(!partial_palette.has_value());
  }

  TEST_CASE("Palette constants validation")
  {
    // Verify palette constants are consistent
    CHECK(art2img::core::palette_color_count == 256);
    CHECK(art2img::core::palette_component_count ==
          art2img::core::palette_color_count * 3);
    CHECK(art2img::core::shade_table_size ==
          art2img::core::palette_color_count);
    CHECK(art2img::core::translucent_table_size == 65536);  // 256 * 256
  }

  TEST_CASE("Palette with additional data")
  {
    const auto test_assets_dir = std::filesystem::path{__FILE__}
                                     .parent_path()
                                     .parent_path()
                                     .parent_path() /
                                 "assets";
    const auto palette_file = test_assets_dir / "PALETTE.DAT";

    auto file_data = art2img::adapters::read_binary_file(palette_file);
    REQUIRE(file_data.has_value());

    auto palette = art2img::core::load_palette(*file_data);
    REQUIRE(palette.has_value());

    // Check if palette has additional data beyond basic RGB
    if (file_data->size() > art2img::core::palette_component_count) {
      // The palette loader should handle additional data (shade tables, etc.)
      CHECK(palette->shade_table_count >= 0);

      if (palette->shade_table_count > 0) {
        CHECK(!palette->shade_tables.empty());
        CHECK(palette->shade_tables.size() >=
              static_cast<std::size_t>(palette->shade_table_count) *
                  art2img::core::shade_table_size);
      }
    }
  }
}