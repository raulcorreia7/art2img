#pragma once

#include "art_file.hpp"
#include "palette.hpp"
#include "exceptions.hpp"
#include <vector>
#include <string>

namespace art2img {

class TgaWriter {
public:
    struct TgaHeader {
        uint8_t id_length;           // Image ID field length (0)
        uint8_t color_map_type;      // Color map type (1 = palette)
        uint8_t image_type;          // Image type (1 = color mapped)
        uint16_t color_map_start;    // First color map entry (0)
        uint16_t color_map_length;   // Color map length (256)
        uint8_t color_map_depth;     // Color map entry size (24 bits)
        uint16_t x_origin;           // X origin (0)
        uint16_t y_origin;           // Y origin (0)
        uint16_t width;              // Image width
        uint16_t height;             // Image height
        uint8_t pixel_depth;         // Bits per pixel (8)
        uint8_t image_descriptor;    // Image descriptor (0x00)
        
        std::vector<uint8_t> serialize() const;
    };
    
    // File-based operations (vector version)
    static bool write_tga(const std::string& filename,
                         const Palette& palette,
                         const ArtFile::Tile& tile,
                         const std::vector<uint8_t>& pixel_data);

    // File-based operations (raw pointer version)
    static bool write_tga(const std::string& filename,
                         const Palette& palette,
                         const ArtFile::Tile& tile,
                         const uint8_t* pixel_data,
                         size_t pixel_data_size);

    // Memory-based operations (vector version)
    static bool write_tga_to_memory(std::vector<uint8_t>& output,
                                   const Palette& palette,
                                   const ArtFile::Tile& tile,
                                   const std::vector<uint8_t>& pixel_data);

    // Memory-based operations (raw pointer version)
    static bool write_tga_to_memory(std::vector<uint8_t>& output,
                                   const Palette& palette,
                                   const ArtFile::Tile& tile,
                                   const uint8_t* pixel_data,
                                   size_t pixel_data_size);
    
    static TgaHeader create_header(uint16_t width, uint16_t height);
    
private:
    static void write_little_endian_uint16(uint16_t value, std::vector<uint8_t>& buffer, size_t offset);
};

} // namespace art2img