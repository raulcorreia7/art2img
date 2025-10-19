#include <doctest/doctest.h>
#include <art2img/types.hpp>

TEST_SUITE("types::constants") {
    
    TEST_CASE("Palette constants are correct") {
        using namespace art2img::constants;
        
        CHECK(PALETTE_SIZE == 256);
        CHECK(COLOR_COMPONENTS == 3);
        CHECK(PALETTE_DATA_SIZE == 768); // 256 * 3
        CHECK(PALETTE_BITS_PER_COMPONENT == 6);
        CHECK(PALETTE_COMPONENT_MAX == 63);
        CHECK(PALETTE_SCALE_FACTOR == 4);
    }
    
    TEST_CASE("Shade table constants are correct") {
        using namespace art2img::constants;
        
        CHECK(SHADE_TABLE_COUNT == 32);
        CHECK(SHADE_TABLE_SIZE == 256);
        CHECK(SHADE_TABLE_TOTAL_ENTRIES == 8192); // 32 * 256
    }
    
    TEST_CASE("Translucent table constants are correct") {
        using namespace art2img::constants;
        
        CHECK(TRANSLUCENT_TABLE_SIZE == 65536);
    }
    
    TEST_CASE("Tile constants are correct") {
        using namespace art2img::constants;
        
        CHECK(MAX_TILE_WIDTH == 32767);
        CHECK(MAX_TILE_HEIGHT == 32767);
        CHECK(MAX_TILE_DIMENSION == 32767);
    }
    
    TEST_CASE("RGBA constants are correct") {
        using namespace art2img::constants;
        
        CHECK(RGBA_BYTES_PER_PIXEL == 4);
        CHECK(RGBA_CHANNEL_COUNT == 4);
    }
}

TEST_SUITE("types::type_aliases") {
    
    TEST_CASE("Type aliases are correctly defined") {
        using namespace art2img::types;
        
        // Check that types are properly defined
        static_assert(std::is_same_v<byte, std::byte>);
        static_assert(std::is_same_v<u8, std::uint8_t>);
        static_assert(std::is_same_v<u16, std::uint16_t>);
        static_assert(std::is_same_v<u32, std::uint32_t>);
        static_assert(std::is_same_v<u64, std::uint64_t>);
        
        // Check span types (these should compile without error)
        // Note: We can't easily test span types at runtime, but we can ensure they compile
        [[maybe_unused]] byte_span test_byte_span;
        [[maybe_unused]] mutable_byte_span test_mutable_byte_span;
        [[maybe_unused]] u8_span test_u8_span;
        [[maybe_unused]] mutable_u8_span test_mutable_u8_span;
        
        // If we reach here, the span types compiled successfully
        CHECK(true);
    }
}