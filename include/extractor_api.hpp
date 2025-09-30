#pragma once

#include "art_file.hpp"
#include "palette.hpp"
#include "png_writer.hpp"
#include "tga_writer.hpp"
#include <vector>
#include <string>
#include <memory>

namespace art2image {

struct ExtractionResult {
    bool success;
    std::string error_message;
    std::vector<uint8_t> image_data;
    std::string format; // "png" or "tga"
    uint32_t tile_index;
    uint16_t width;
    uint16_t height;
    
    // Animation data
    uint32_t anim_frames;
    uint32_t anim_type;
    int8_t x_offset;
    int8_t y_offset;
    uint32_t anim_speed;
    uint32_t other_flags;
};

class ExtractorAPI {
public:
    ExtractorAPI();
    
    // File-based operations
    bool load_art_file(const std::string& filename);
    bool load_palette_file(const std::string& filename);
    
    // Memory-based operations
    bool load_art_from_memory(const uint8_t* data, size_t size);
    bool load_palette_from_memory(const uint8_t* data, size_t size);
    
    // Set default palettes
    void set_duke3d_default_palette();
    void set_blood_default_palette();
    
    // Extraction methods
    ExtractionResult extract_tile(uint32_t tile_index, PngWriter::Options options = PngWriter::Options());
    ExtractionResult extract_tile_tga(uint32_t tile_index);
    
    // Batch extraction
    std::vector<ExtractionResult> extract_all_tiles(PngWriter::Options options = PngWriter::Options());
    std::vector<ExtractionResult> extract_all_tiles_tga();
    
    // Accessors
    bool is_art_loaded() const { return art_file_ != nullptr; }
    bool is_palette_loaded() const { return palette_ != nullptr; }
    uint32_t get_tile_count() const { return art_file_ ? static_cast<uint32_t>(art_file_->tiles().size()) : 0; }
    
private:
    std::unique_ptr<ArtFile> art_file_;
    std::unique_ptr<Palette> palette_;
};

} // namespace art2image