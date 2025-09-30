#include "extractor_api.hpp"
#include <iostream>

namespace art2image {

ExtractorAPI::ExtractorAPI() {
    // Initialize with default palette
    palette_ = std::make_unique<Palette>();
}

bool ExtractorAPI::load_art_file(const std::string& filename) {
    art_file_ = std::make_unique<ArtFile>();
    if (!art_file_->open(filename)) {
        art_file_.reset();
        return false;
    }
    return true;
}

bool ExtractorAPI::load_palette_file(const std::string& filename) {
    if (!palette_) {
        palette_ = std::make_unique<Palette>();
    }
    return palette_->load_from_file(filename);
}

bool ExtractorAPI::load_art_from_memory(const uint8_t* data, size_t size) {
    art_file_ = std::make_unique<ArtFile>();
    if (!art_file_->load_from_memory(data, size)) {
        art_file_.reset();
        return false;
    }
    return true;
}

bool ExtractorAPI::load_palette_from_memory(const uint8_t* data, size_t size) {
    if (!palette_) {
        palette_ = std::make_unique<Palette>();
    }
    return palette_->load_from_memory(data, size);
}

void ExtractorAPI::set_duke3d_default_palette() {
    if (!palette_) {
        palette_ = std::make_unique<Palette>();
    }
    palette_->load_duke3d_default();
}

void ExtractorAPI::set_blood_default_palette() {
    if (!palette_) {
        palette_ = std::make_unique<Palette>();
    }
    palette_->load_blood_default();
}

ExtractionResult ExtractorAPI::extract_tile(uint32_t tile_index, PngWriter::Options options) {
    ExtractionResult result;
    result.success = false;
    result.tile_index = tile_index;
    result.format = "png";
    
    if (!art_file_ || !palette_) {
        result.error_message = "ART file or palette not loaded";
        return result;
    }
    
    const auto& tiles = art_file_->tiles();
    if (tile_index >= tiles.size()) {
        result.error_message = "Tile index out of range";
        return result;
    }
    
    const auto& tile = tiles[tile_index];
    result.width = tile.width;
    result.height = tile.height;
    
    // Set animation data
    result.anim_frames = tile.anim_frames();
    result.anim_type = tile.anim_type();
    result.x_offset = tile.x_offset();
    result.y_offset = tile.y_offset();
    result.anim_speed = tile.anim_speed();
    result.other_flags = tile.other_flags();
    
    if (tile.is_empty()) {
        result.success = true;
        return result;
    }
    
    // Read tile data
    std::vector<uint8_t> pixel_data;
    if (!art_file_->read_tile_data_from_memory(tile_index, pixel_data)) {
        result.error_message = "Failed to read tile data";
        return result;
    }
    
    // Extract to PNG in memory
    if (!PngWriter::write_png_to_memory(result.image_data, *palette_, tile, pixel_data, options)) {
        result.error_message = "Failed to write PNG to memory";
        return result;
    }
    
    result.success = true;
    return result;
}

ExtractionResult ExtractorAPI::extract_tile_tga(uint32_t tile_index) {
    ExtractionResult result;
    result.success = false;
    result.tile_index = tile_index;
    result.format = "tga";
    
    if (!art_file_ || !palette_) {
        result.error_message = "ART file or palette not loaded";
        return result;
    }
    
    const auto& tiles = art_file_->tiles();
    if (tile_index >= tiles.size()) {
        result.error_message = "Tile index out of range";
        return result;
    }
    
    const auto& tile = tiles[tile_index];
    result.width = tile.width;
    result.height = tile.height;
    
    // Set animation data
    result.anim_frames = tile.anim_frames();
    result.anim_type = tile.anim_type();
    result.x_offset = tile.x_offset();
    result.y_offset = tile.y_offset();
    result.anim_speed = tile.anim_speed();
    result.other_flags = tile.other_flags();
    
    if (tile.is_empty()) {
        result.success = true;
        return result;
    }
    
    // Read tile data
    std::vector<uint8_t> pixel_data;
    if (!art_file_->read_tile_data_from_memory(tile_index, pixel_data)) {
        result.error_message = "Failed to read tile data";
        return result;
    }
    
    // Extract to TGA in memory
    if (!TgaWriter::write_tga_to_memory(result.image_data, *palette_, tile, pixel_data)) {
        result.error_message = "Failed to write TGA to memory";
        return result;
    }
    
    result.success = true;
    return result;
}

std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles(PngWriter::Options options) {
    std::vector<ExtractionResult> results;
    
    if (!art_file_ || !palette_) {
        ExtractionResult result;
        result.success = false;
        result.error_message = "ART file or palette not loaded";
        results.push_back(result);
        return results;
    }
    
    uint32_t tile_count = static_cast<uint32_t>(art_file_->tiles().size());
    results.reserve(tile_count);
    
    for (uint32_t i = 0; i < tile_count; ++i) {
        results.push_back(extract_tile(i, options));
    }
    
    return results;
}

std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_tga() {
    std::vector<ExtractionResult> results;
    
    if (!art_file_ || !palette_) {
        ExtractionResult result;
        result.success = false;
        result.error_message = "ART file or palette not loaded";
        results.push_back(result);
        return results;
    }
    
    uint32_t tile_count = static_cast<uint32_t>(art_file_->tiles().size());
    results.reserve(tile_count);
    
    for (uint32_t i = 0; i < tile_count; ++i) {
        results.push_back(extract_tile_tga(i));
    }
    
    return results;
}

} // namespace art2image