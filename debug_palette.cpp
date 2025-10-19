#include <art2img/palette.hpp>
#include <iostream>
#include <iomanip>

using namespace art2img;

int main() {
    // Create test palette like in the test
    Palette palette;
    
    // Index 0: Black
    palette.data[0] = 0;   // R
    palette.data[1] = 0;   // G
    palette.data[2] = 0;   // B
    
    // Index 1: Red
    palette.data[3] = 63;  // R
    palette.data[4] = 0;   // G
    palette.data[5] = 0;   // B
    
    // Test palette entry conversion
    std::cout << "Palette entry 0 (black):" << std::endl;
    auto [r0, g0, b0] = palette_entry_to_rgb(palette, 0);
    std::cout << "  RGB: (" << (int)r0 << ", " << (int)g0 << ", " << (int)b0 << ")" << std::endl;
    
    u32 bgra0 = palette_entry_to_rgba(palette, 0);
    std::cout << "  BGRA: 0x" << std::hex << std::setfill('0') << std::setw(8) << bgra0 << std::dec << std::endl;
    std::cout << "  B: " << (int)(bgra0 & 0xFF) << std::endl;
    std::cout << "  G: " << (int)((bgra0 >> 8) & 0xFF) << std::endl;
    std::cout << "  R: " << (int)((bgra0 >> 16) & 0xFF) << std::endl;
    std::cout << "  A: " << (int)((bgra0 >> 24) & 0xFF) << std::endl;
    
    std::cout << "\nPalette entry 1 (red):" << std::endl;
    auto [r1, g1, b1] = palette_entry_to_rgb(palette, 1);
    std::cout << "  RGB: (" << (int)r1 << ", " << (int)g1 << ", " << (int)b1 << ")" << std::endl;
    
    u32 bgra1 = palette_entry_to_rgba(palette, 1);
    std::cout << "  BGRA: 0x" << std::hex << std::setfill('0') << std::setw(8) << bgra1 << std::dec << std::endl;
    std::cout << "  B: " << (int)(bgra1 & 0xFF) << std::endl;
    std::cout << "  G: " << (int)((bgra1 >> 8) & 0xFF) << std::endl;
    std::cout << "  R: " << (int)((bgra1 >> 16) & 0xFF) << std::endl;
    std::cout << "  A: " << (int)((bgra1 >> 24) & 0xFF) << std::endl;
    
    return 0;
}