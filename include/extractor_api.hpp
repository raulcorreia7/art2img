#pragma once

#include "art_file.hpp"
#include "palette.hpp"
#include "png_writer.hpp"
#include "tga_writer.hpp"
#include "exceptions.hpp"
#include <vector>
#include <string>
#include <memory>

namespace art2img {

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

// Zero-copy view structures for parallel processing
struct ArtView {
    const uint8_t* art_data;           // Pointer to original ART memory
    size_t art_size;                   // Size of ART data
    const Palette* palette;            // Palette for color conversion
    ArtFile::Header header;            // ART file header
    std::vector<ArtFile::Tile> tiles;  // Tile metadata

    // On-demand ImageView creation
    size_t image_count() const { return tiles.size(); }
    ArtFile::Tile get_tile(uint32_t tile_index) const {
        if (tile_index >= tiles.size()) {
            throw ArtException("Tile index out of range");
        }
        return tiles[tile_index];
    }
};

struct ImageView {
    const ArtView* parent;             // Parent ArtView
    uint32_t tile_index;               // Index of this tile

    // Direct memory access
    const uint8_t* pixel_data() const {
        if (!parent || tile_index >= parent->tiles.size()) {
            throw ArtException("Invalid ImageView state");
        }
        const auto& tile = parent->tiles[tile_index];
        if (tile.is_empty()) {
            return nullptr;
        }
        if (tile.offset + tile.size() > parent->art_size) {
            throw ArtException("Tile data extends beyond buffer size");
        }
        return parent->art_data + tile.offset;
    }

    uint16_t width() const { return parent ? parent->tiles[tile_index].width : 0; }
    uint16_t height() const { return parent ? parent->tiles[tile_index].height : 0; }
    size_t size() const { return static_cast<size_t>(width()) * height(); }

    // Animation data accessors
    uint32_t anim_frames() const { return parent ? parent->tiles[tile_index].anim_frames() : 0; }
    uint32_t anim_type() const { return parent ? parent->tiles[tile_index].anim_type() : 0; }
    int8_t x_offset() const { return parent ? parent->tiles[tile_index].x_offset() : 0; }
    int8_t y_offset() const { return parent ? parent->tiles[tile_index].y_offset() : 0; }
    uint32_t anim_speed() const { return parent ? parent->tiles[tile_index].anim_speed() : 0; }
    uint32_t other_flags() const { return parent ? parent->tiles[tile_index].other_flags() : 0; }

    // PNG saving (conversion + write happens here)
    bool save_to_png(const std::string& path, PngWriter::Options options = PngWriter::Options()) const;
    bool save_to_png(const std::string& path) const { return save_to_png(path, PngWriter::Options()); }

    std::vector<uint8_t> extract_to_png(PngWriter::Options options = PngWriter::Options()) const;
    std::vector<uint8_t> extract_to_png() const { return extract_to_png(PngWriter::Options()); }

    // TGA saving
    bool save_to_tga(const std::string& path) const;
    std::vector<uint8_t> extract_to_tga() const;
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

    // Zero-copy view access for parallel processing
    ArtView get_art_view() const;
    
private:
    std::unique_ptr<ArtFile> art_file_;
    std::unique_ptr<Palette> palette_;
};

} // namespace art2img