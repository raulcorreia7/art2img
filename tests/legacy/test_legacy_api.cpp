// Tests for the legacy API compatibility layer

#include <doctest/doctest.h>
#include <art2img/legacy_api.hpp>
#include <filesystem>
#include <fstream>
#include <vector>

using namespace art2img;

// Test fixtures and helpers
class LegacyApiTestFixture {
public:
    std::filesystem::path test_data_dir = "tests/assets";
    std::filesystem::path art_file = test_data_dir / "TILES000.ART";
    std::filesystem::path palette_file = test_data_dir / "PALETTE.DAT";
    
    // Helper to create a simple test ART file in memory
    std::vector<uint8_t> create_simple_art_data() {
        // Create a minimal ART file with one 16x16 tile
        std::vector<uint8_t> data;
        
        // Header (16 bytes)
        data.push_back(1);  // version
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back(0);  // start_tile
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back(0);  // end_tile
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        data.push_back(1);  // num_tiles
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Tile entry (16 bytes)
        data.push_back(16); // width
        data.push_back(0);
        data.push_back(16); // height
        data.push_back(0);
        data.push_back(0);  // anim_data
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(32); // offset (after header + tile entries)
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Padding (4 bytes)
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        data.push_back(0);
        
        // Tile data (256 pixels)
        for (int i = 0; i < 256; ++i) {
            data.push_back(static_cast<uint8_t>(i % 256));
        }
        
        return data;
    }
    
    // Helper to create a simple test palette in memory
    std::vector<uint8_t> create_simple_palette_data() {
        std::vector<uint8_t> data(256 * 3);
        
        // Create a simple gradient palette
        for (int i = 0; i < 256; ++i) {
            data[i * 3] = i;     // Red
            data[i * 3 + 1] = i; // Green
            data[i * 3 + 2] = i; // Blue
        }
        
        return data;
    }
};

TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - ArtFile") {
    SUBCASE("Default constructor") {
        ArtFile art_file;
        CHECK_FALSE(art_file.is_open());
        CHECK_EQ(art_file.tiles().size(), 0);
    }
    
    SUBCASE("Load from memory") {
        auto art_data = create_simple_art_data();
        ArtFile art_file;
        
        CHECK(art_file.load(art_data.data(), art_data.size()));
        CHECK(art_file.is_open());
        CHECK_EQ(art_file.tiles().size(), 1);
        CHECK_EQ(art_file.header().version, 1);
        CHECK_EQ(art_file.header().num_tiles, 1);
        
        const auto& tile = art_file.tiles()[0];
        CHECK_EQ(tile.width, 16);
        CHECK_EQ(tile.height, 16);
        CHECK_FALSE(tile.is_empty());
    }
    
    SUBCASE("Read tile data") {
        auto art_data = create_simple_art_data();
        ArtFile art_file;
        art_file.load(art_data.data(), art_data.size());
        
        std::vector<uint8_t> tile_data;
        CHECK(art_file.read_tile_data(0, tile_data));
        CHECK_EQ(tile_data.size(), 256);
        
        // Check first few pixels
        CHECK_EQ(tile_data[0], 0);
        CHECK_EQ(tile_data[1], 1);
        CHECK_EQ(tile_data[2], 2);
    }
}

TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - Palette") {
    SUBCASE("Default constructor") {
        Palette palette;
        CHECK_FALSE(palette.is_loaded());
        CHECK_EQ(palette.raw_data().size(), 0);
    }
    
    SUBCASE("Load from memory") {
        auto palette_data = create_simple_palette_data();
        Palette palette;
        
        CHECK(palette.load_from_memory(palette_data.data(), palette_data.size()));
        CHECK(palette.is_loaded());
        CHECK_EQ(palette.raw_data().size(), 256 * 3);
        
        // Check first few colors
        CHECK_EQ(palette.get_red(0), 0);
        CHECK_EQ(palette.get_green(0), 0);
        CHECK_EQ(palette.get_blue(0), 0);
        
        CHECK_EQ(palette.get_red(1), 1);
        CHECK_EQ(palette.get_green(1), 1);
        CHECK_EQ(palette.get_blue(1), 1);
    }
    
    SUBCASE("Get BGR data") {
        auto palette_data = create_simple_palette_data();
        Palette palette;
        palette.load_from_memory(palette_data.data(), palette_data.size());
        
        auto bgr_data = palette.get_bgr_data();
        CHECK_EQ(bgr_data.size(), 256 * 3);
        
        // Check BGR order (should be reversed from RGB)
        CHECK_EQ(bgr_data[0], 0);   // Blue
        CHECK_EQ(bgr_data[1], 0);   // Green  
        CHECK_EQ(bgr_data[2], 0);   // Red
    }
}

TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - ImageWriter") {
    SUBCASE("Is magenta detection") {
        CHECK(ImageWriter::is_magenta(255, 0, 255));
        CHECK(ImageWriter::is_magenta(250, 5, 250));
        CHECK_FALSE(ImageWriter::is_magenta(254, 6, 255));
        CHECK_FALSE(ImageWriter::is_magenta(255, 255, 255));
    }
}

TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - ExtractorAPI") {
    SUBCASE("Default constructor") {
        ExtractorAPI api;
        CHECK_FALSE(api.is_art_loaded());
        CHECK_FALSE(api.is_palette_loaded());
        CHECK_EQ(api.get_tile_count(), 0);
    }
    
    SUBCASE("Load from memory") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        CHECK(api.load_art_from_memory(art_data.data(), art_data.size()));
        CHECK(api.load_palette_from_memory(palette_data.data(), palette_data.size()));
        
        CHECK(api.is_art_loaded());
        CHECK(api.is_palette_loaded());
        CHECK_EQ(api.get_tile_count(), 1);
    }
    
    SUBCASE("Extract tile") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        api.load_palette_from_memory(palette_data.data(), palette_data.size());
        
        auto result = api.extract_tile(0, ImageFormat::PNG);
        CHECK(result.success);
        CHECK_EQ(result.width, 16);
        CHECK_EQ(result.height, 16);
        CHECK_EQ(result.format, "png");
        CHECK_FALSE(result.image_data.empty());
    }
    
    SUBCASE("Extract all tiles") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        api.load_palette_from_memory(palette_data.data(), palette_data.size());
        
        auto results = api.extract_all_tiles(ImageFormat::PNG);
        CHECK_EQ(results.size(), 1);
        CHECK(results[0].success);
        CHECK_EQ(results[0].width, 16);
        CHECK_EQ(results[0].height, 16);
    }
    
    SUBCASE("Get art view") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        api.load_palette_from_memory(palette_data.data(), palette_data.size());
        
        auto view = api.get_art_view();
        CHECK_EQ(view.tiles.size(), 1);
        CHECK_EQ(view.header.num_tiles, 1);
        CHECK(view.art_data != nullptr);
        CHECK(view.palette != nullptr);
    }
}

TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - ImageView") {
    SUBCASE("Create from art view") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        api.load_palette_from_memory(palette_data.data(), palette_data.size());
        
        auto art_view = api.get_art_view();
        ImageView image_view;
        image_view.parent = &art_view;
        image_view.tile_index = 0;
        
        CHECK_EQ(image_view.width(), 16);
        CHECK_EQ(image_view.height(), 16);
        CHECK_EQ(image_view.size(), 256);
        CHECK(image_view.pixel_data() != nullptr);
    }
}

TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - Integration") {
    SUBCASE("Full extraction workflow") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        CHECK(api.load_art_from_memory(art_data.data(), art_data.size()));
        CHECK(api.load_palette_from_memory(palette_data.data(), palette_data.size()));
        
        // Extract as PNG
        auto png_result = api.extract_tile_png(0);
        CHECK(png_result.success);
        CHECK_EQ(png_result.format, "png");
        
        // Extract as TGA
        auto tga_result = api.extract_tile_tga(0);
        CHECK(tga_result.success);
        CHECK_EQ(tga_result.format, "tga");
        
        // Extract as BMP
        auto bmp_result = api.extract_tile_bmp(0);
        CHECK(bmp_result.success);
        CHECK_EQ(bmp_result.format, "bmp");
    }
    
    SUBCASE("Animation data handling") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        // Modify art data to include animation
        art_data[20] = 0x42; // Set anim_data to have 2 frames, type 1
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        api.load_palette_from_memory(palette_data.data(), palette_data.size());
        
        auto result = api.extract_tile(0, ImageFormat::PNG);
        CHECK(result.success);
        CHECK_EQ(result.anim_frames, 2);
        CHECK_EQ(result.anim_type, 1);
    }
}

// Test error conditions
TEST_CASE_FIXTURE(LegacyApiTestFixture, "Legacy API - Error Handling") {
    SUBCASE("Invalid tile index") {
        auto art_data = create_simple_art_data();
        auto palette_data = create_simple_palette_data();
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        api.load_palette_from_memory(palette_data.data(), palette_data.size());
        
        auto result = api.extract_tile(999, ImageFormat::PNG);
        CHECK_FALSE(result.success);
        CHECK_FALSE(result.error_message.empty());
    }
    
    SUBCASE("Missing palette") {
        auto art_data = create_simple_art_data();
        
        ExtractorAPI api;
        api.load_art_from_memory(art_data.data(), art_data.size());
        
        auto result = api.extract_tile(0, ImageFormat::PNG);
        CHECK_FALSE(result.success);
        CHECK_FALSE(result.error_message.empty());
    }
    
    SUBCASE("Invalid art data") {
        std::vector<uint8_t> invalid_data = {0xFF, 0xFF, 0xFF, 0xFF};
        
        ExtractorAPI api;
        CHECK_FALSE(api.load_art_from_memory(invalid_data.data(), invalid_data.size()));
    }
}