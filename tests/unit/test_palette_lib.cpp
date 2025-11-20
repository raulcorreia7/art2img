#include <doctest/doctest.h>
#include "art2img/palette.hpp"
#include <vector>

TEST_CASE("parse_palette") {
    // Create a temporary palette buffer
    std::vector<std::byte> buffer;
    buffer.reserve(768);
    
    // Write 256 colors
    for (int i = 0; i < 256; ++i) {
        // Use values that will scale predictably
        // 63 -> 255 (111111 -> 11111111)
        // 0 -> 0
        uint8_t val = (i % 64); 
        std::byte b = std::byte{val};
        buffer.push_back(b); // R
        buffer.push_back(b); // G
        buffer.push_back(b); // B
    }

    auto result = art2img::parse_palette(buffer);
    CHECK(result.has_value());
    
    if (result.has_value()) {
        const auto& palette = result.value();
        CHECK(palette.colors.size() == 256);
        
        // Check scaling
        // 0 -> 0
        // 63 -> 255
        // 1 -> 4 | 0 = 4
        
        // Check index 0 (val 0)
        CHECK(palette.colors[0].r == 0);
        CHECK(palette.colors[0].a == 255);

        // Check index 63 (val 63)
        // 63 = 0x3F (00111111)
        // (0x3F << 2) = 0xFC (11111100)
        // (0x3F >> 4) = 0x03 (00000011)
        // 0xFC | 0x03 = 0xFF (255)
        CHECK(palette.colors[63].r == 255);
        
        // Check index 255 (alpha should be 0)
        CHECK(palette.colors[255].a == 0);
    }
}

TEST_CASE("parse_palette too small") {
    std::vector<std::byte> buffer = {std::byte{'a'}, std::byte{'b'}, std::byte{'c'}};

    auto result = art2img::parse_palette(buffer);
    CHECK(!result.has_value());
}
