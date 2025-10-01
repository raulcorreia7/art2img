#include "tga_writer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

namespace art2img {

bool TgaWriter::write_tga(const std::string& filename,
                         const Palette& palette,
                         const ArtFile::Tile& tile,
                         const std::vector<uint8_t>& pixel_data) {
    if (tile.is_empty()) {
        return true; // Skip empty tiles
    }
    
    if (pixel_data.size() != tile.size()) {
        std::cerr << "Error: Pixel data size mismatch for tile " << filename << std::endl;
        return false;
    }
    
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot create file '" << filename << "'" << std::endl;
        return false;
    }
    
    // Create and write TGA header
    TgaHeader header = create_header(tile.width, tile.height);
    std::vector<uint8_t> header_data = header.serialize();
    
    if (!file.write(reinterpret_cast<const char*>(header_data.data()), header_data.size())) {
        std::cerr << "Error: Cannot write TGA header to '" << filename << "'" << std::endl;
        return false;
    }
    
    // Write palette
    const std::vector<uint8_t>& palette_data = palette.data();
    if (!file.write(reinterpret_cast<const char*>(palette_data.data()), palette_data.size())) {
        std::cerr << "Error: Cannot write palette to '" << filename << "'" << std::endl;
        return false;
    }
    
    // Write pixel data (ART stores by columns, TGA by rows from bottom)
    for (int32_t y = tile.height - 1; y >= 0; --y) {
        for (int32_t x = 0; x < tile.width; ++x) {
            // ART format: pixels stored by columns (y + x * height)
            uint8_t pixel = pixel_data[y + x * tile.height];
            if (!file.write(reinterpret_cast<const char*>(&pixel), 1)) {
                std::cerr << "Error: Cannot write pixel data to '" << filename << "'" << std::endl;
                return false;
            }
        }
    }
    
    return true;
}

bool TgaWriter::write_tga_to_memory(std::vector<uint8_t>& output,
                                   const Palette& palette,
                                   const ArtFile::Tile& tile,
                                   const std::vector<uint8_t>& pixel_data) {
    output.clear();
    
    if (tile.is_empty()) {
        return true; // Skip empty tiles
    }
    
    if (pixel_data.size() != tile.size()) {
        std::cerr << "Error: Pixel data size mismatch for tile" << std::endl;
        return false;
    }
    
    // Create TGA header
    TgaHeader header = create_header(tile.width, tile.height);
    std::vector<uint8_t> header_data = header.serialize();
    
    // Reserve space for the output buffer
    output.reserve(18 + 768 + tile.size()); // header + palette + pixel data
    
    // Write TGA header
    output.insert(output.end(), header_data.begin(), header_data.end());
    
    // Write palette
    const std::vector<uint8_t>& palette_data = palette.data();
    output.insert(output.end(), palette_data.begin(), palette_data.end());
    
    // Write pixel data (ART stores by columns, TGA by rows from bottom)
    for (int32_t y = tile.height - 1; y >= 0; --y) {
        for (int32_t x = 0; x < tile.width; ++x) {
            // ART format: pixels stored by columns (y + x * height)
            uint8_t pixel = pixel_data[y + x * tile.height];
            output.push_back(pixel);
        }
    }
    
    return true;
}

TgaWriter::TgaHeader TgaWriter::create_header(uint16_t width, uint16_t height) {
    TgaHeader header;
    header.id_length = 0;
    header.color_map_type = 1;        // Palette present
    header.image_type = 1;            // Color mapped image
    header.color_map_start = 0;       // First palette entry
    header.color_map_length = 256;    // 256 colors
    header.color_map_depth = 24;      // 24-bit palette entries
    header.x_origin = 0;
    header.y_origin = 0;
    header.width = width;
    header.height = height;
    header.pixel_depth = 8;           // 8-bit indexed color
    header.image_descriptor = 0x00;   // Bottom-left origin, no alpha
    
    return header;
}

std::vector<uint8_t> TgaWriter::TgaHeader::serialize() const {
    std::vector<uint8_t> buffer(18); // TGA header is 18 bytes
    
    buffer[0] = id_length;
    buffer[1] = color_map_type;
    buffer[2] = image_type;
    
    // Color map specification (5 bytes)
    write_little_endian_uint16(color_map_start, buffer, 3);
    write_little_endian_uint16(color_map_length, buffer, 5);
    buffer[7] = color_map_depth;
    
    // Image specification (10 bytes)
    write_little_endian_uint16(x_origin, buffer, 8);
    write_little_endian_uint16(y_origin, buffer, 10);
    write_little_endian_uint16(width, buffer, 12);
    write_little_endian_uint16(height, buffer, 14);
    buffer[16] = pixel_depth;
    buffer[17] = image_descriptor;
    
    return buffer;
}

void TgaWriter::write_little_endian_uint16(uint16_t value, std::vector<uint8_t>& buffer, size_t offset) {
    buffer[offset] = static_cast<uint8_t>(value & 0xFF);
    buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

} // namespace art2img