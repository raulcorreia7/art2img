#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <fstream>
#include <memory>

namespace art2image {

class ArtFile {
public:
    struct Header {
        uint32_t version;
        uint32_t start_tile;
        uint32_t end_tile;
        uint32_t num_tiles;
        
        bool is_valid() const {
            return version == 1 && end_tile >= start_tile;
        }
    };
    
    struct Tile {
        uint16_t width;
        uint16_t height;
        uint32_t anim_data;
        uint32_t offset;
        
        uint32_t size() const { return static_cast<uint32_t>(width) * height; }
        bool is_empty() const { return size() == 0; }
        
        // Animation data accessors
        uint32_t anim_frames() const { return anim_data & 0x3F; }
        uint32_t anim_type() const { return (anim_data >> 6) & 0x03; }
        int8_t x_offset() const { return static_cast<int8_t>((anim_data >> 8) & 0xFF); }
        int8_t y_offset() const { return static_cast<int8_t>((anim_data >> 16) & 0xFF); }
        uint32_t anim_speed() const { return (anim_data >> 24) & 0x0F; }
        uint32_t other_flags() const { return anim_data >> 28; }
    };
    
    ArtFile() = default;
    explicit ArtFile(const std::string& filename);
    
    // Non-copyable, movable
    ArtFile(const ArtFile&) = delete;
    ArtFile& operator=(const ArtFile&) = delete;
    ArtFile(ArtFile&&) = default;
    ArtFile& operator=(ArtFile&&) = default;
    
    bool open(const std::string& filename);
    void close();
    
    bool read_header();
    bool read_tile_data(uint32_t index, std::vector<uint8_t>& buffer);
    
    // Accessors
    const Header& header() const { return header_; }
    const std::vector<Tile>& tiles() const { return tiles_; }
    bool is_open() const { return file_.is_open(); }
    const std::string& filename() const { return filename_; }
    
private:
    bool read_tile_metadata();
    bool calculate_offsets();
    
    uint16_t read_little_endian_uint16(std::ifstream& file);
    uint32_t read_little_endian_uint32(std::ifstream& file);
    
    std::string filename_;
    std::ifstream file_;
    Header header_;
    std::vector<Tile> tiles_;
};

} // namespace art2image