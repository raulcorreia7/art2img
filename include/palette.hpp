#pragma once

#include <cstdint>
#include <vector>
#include <string>

namespace art2img {

class Palette {
public:
    static constexpr size_t SIZE = 256 * 3; // 256 colors, 3 components each
    
    Palette();
    
    // Non-copyable, movable
    Palette(const Palette&) = delete;
    Palette& operator=(const Palette&) = delete;
    Palette(Palette&&) = default;
    Palette& operator=(Palette&&) = default;
    
    // File-based operations
    bool load_from_file(const std::string& filename);
    void load_duke3d_default();
    void load_blood_default();
    
    // Memory-based operations
    bool load_from_memory(const uint8_t* data, size_t size);
    
    // Convert palette to TGA format (RGB -> BGR and scale)
    void convert_to_tga_format();
    
    // Accessors
    const std::vector<uint8_t>& data() const { return data_; }
    bool is_loaded() const { return !data_.empty(); }
    
    // Get specific color component
    uint8_t get_red(size_t index) const;
    uint8_t get_green(size_t index) const;
    uint8_t get_blue(size_t index) const;
    
private:
    std::vector<uint8_t> data_;
};

} // namespace art2img