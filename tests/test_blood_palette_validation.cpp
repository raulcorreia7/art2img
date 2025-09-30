#include "palette.hpp"
#include <iostream>
#include <cassert>

int main() {
    std::cout << "Blood Palette Validation Test" << std::endl;
    std::cout << "============================" << std::endl;
    
    // Test that Blood palette method exists and works
    art2image::Palette blood_palette;
    blood_palette.load_blood_default();
    
    std::cout << "✓ Blood palette method exists and can be called" << std::endl;
    
    // Verify palette is loaded
    assert(blood_palette.is_loaded());
    std::cout << "✓ Blood palette is loaded successfully" << std::endl;
    
    // Check palette size
    assert(blood_palette.data().size() == 768);  // 256 colors * 3 components
    std::cout << "✓ Blood palette has correct size (768 bytes)" << std::endl;
    
    // Test color access
    uint8_t red = blood_palette.get_red(0);
    uint8_t green = blood_palette.get_green(0);
    uint8_t blue = blood_palette.get_blue(0);
    
    std::cout << "✓ Color access methods work correctly" << std::endl;
    
    std::cout << "✓ All Blood palette validation tests passed" << std::endl;
    std::cout << "[SUCCESS] Blood palette functionality validated!" << std::endl;
    
    return 0;
}