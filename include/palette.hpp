#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <filesystem>
#include "exceptions.hpp"

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
    bool load_from_file(const std::filesystem::path& filename);
    void load_duke3d_default();
    void load_blood_default();

    // Memory-based operations
    bool load_from_memory(const uint8_t* data, size_t size);

    // Convert palette to TGA format (RGB -> BGR and scale)
    void convert_to_tga_format();

    // Accessors
    const std::vector<uint8_t>& data() const { return bgr_data_; }
    const std::vector<uint8_t>& raw_data() const { return raw_data_; }
    bool is_loaded() const { return loaded_; }

    // Get specific color component
    uint8_t get_red(size_t index) const;
    uint8_t get_green(size_t index) const;
    uint8_t get_blue(size_t index) const;

private:
    void update_bgr_cache();

    std::vector<uint8_t> raw_data_;
    std::vector<uint8_t> bgr_data_;
    bool loaded_ = false;
};

} // namespace art2img
