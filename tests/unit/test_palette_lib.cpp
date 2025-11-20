#include <doctest/doctest.h>
#include "art2img/palette.hpp"
#include <fstream>
#include <filesystem>

TEST_CASE("load_palette") {
    // Create a temporary palette file
    std::filesystem::path temp_path = "temp_palette.dat";
    std::ofstream file(temp_path, std::ios::binary);
    
    // Write 256 colors
    for (int i = 0; i < 256; ++i) {
        // Use values that will scale predictably
        // 63 -> 255 (111111 -> 11111111)
        // 0 -> 0
        uint8_t val = (i % 64); 
        file.put(static_cast<char>(val)); // R
        file.put(static_cast<char>(val)); // G
        file.put(static_cast<char>(val)); // B
    }
    file.close();

    auto result = art2img::load_palette(temp_path);
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

    std::filesystem::remove(temp_path);
}

TEST_CASE("load_palette too small") {
    std::filesystem::path temp_path = "temp_small.dat";
    std::ofstream file(temp_path, std::ios::binary);
    file.write("abc", 3);
    file.close();

    auto result = art2img::load_palette(temp_path);
    CHECK(!result.has_value());
    // We can't easily check the error code enum value without including core headers and dealing with namespaces,
    // but we can check that it failed.
    // If we want to check the code:
    // CHECK(result.error().code == art2img::core::errc::invalid_palette);
    // This requires art2img::core::errc to be visible and comparable.
    // Since Result is std::expected<T, core::Error>, result.error() is core::Error.
    // core::Error has .code which is std::error_code.
    // std::error_code compares with errc if is_error_code_enum is specialized.
    
    std::filesystem::remove(temp_path);
}
