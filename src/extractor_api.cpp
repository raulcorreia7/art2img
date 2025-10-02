#include "extractor_api.hpp"
#include <iostream>

namespace art2img {

ExtractorAPI::ExtractorAPI() {
    // Initialize with default palette
    palette_ = std::make_unique<Palette>();
}

bool ExtractorAPI::load_art_file(const std::filesystem::path& filename) {
    try {
        art_file_ = std::make_unique<ArtFile>(filename);
        return true;
    } catch (const ArtException& e) {
        art_file_.reset();
        throw;
    }
}

bool ExtractorAPI::load_palette_file(const std::filesystem::path& filename) {
    if (!palette_) {
        palette_ = std::make_unique<Palette>();
    }
    try {
        return palette_->load_from_file(filename);
    } catch (const ArtException& e) {
        throw;
    }
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
    
    // Get direct pointer to tile data (zero-copy)
    const uint8_t* pixel_data = nullptr;
    size_t pixel_data_size = 0;

    if (!tile.is_empty()) {
        if (!art_file_->has_data()) {
            result.error_message = "ART data not loaded in memory";
            return result;
        }

        if (tile.offset + tile.size() > art_file_->data_size()) {
            result.error_message = "Tile data extends beyond buffer size";
            return result;
        }

        pixel_data = art_file_->data() + tile.offset;
        pixel_data_size = tile.size();
    }

    // Extract to PNG in memory
    try {
        if (tile.is_empty()) {
            // Empty tile - no image data
            result.success = true;
            return result;
        }

        if (!PngWriter::write_png_to_memory(result.image_data, *palette_, tile, pixel_data, pixel_data_size, options)) {
            result.error_message = "Failed to write PNG to memory";
            return result;
        }
    } catch (const ArtException& e) {
        result.error_message = "Failed to write PNG to memory: " + std::string(e.what());
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
    
    // Get direct pointer to tile data (zero-copy)
    const uint8_t* pixel_data = nullptr;
    size_t pixel_data_size = 0;

    if (!tile.is_empty()) {
        if (!art_file_->has_data()) {
            result.error_message = "ART data not loaded in memory";
            return result;
        }

        if (tile.offset + tile.size() > art_file_->data_size()) {
            result.error_message = "Tile data extends beyond buffer size";
            return result;
        }

        pixel_data = art_file_->data() + tile.offset;
        pixel_data_size = tile.size();
    }

    // Extract to TGA in memory
    try {
        if (tile.is_empty()) {
            // Empty tile - no image data
            result.success = true;
            return result;
        }

        if (!TgaWriter::write_tga_to_memory(result.image_data, *palette_, tile, pixel_data, pixel_data_size)) {
            result.error_message = "Failed to write TGA to memory";
            return result;
        }
    } catch (const ArtException& e) {
        result.error_message = "Failed to write TGA to memory: " + std::string(e.what());
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

ArtView ExtractorAPI::get_art_view() const {
    ArtView view;

    if (!art_file_ || !palette_) {
        throw ArtException("ART file or palette not loaded");
    }

    // Set art data pointer and size
    if (art_file_->is_open() && !art_file_->has_data()) {
        // File-based mode, we need to read the entire file into memory
        throw ArtException("get_art_view() requires memory-based loading, not file-based");
    }

    view.art_data = art_file_->data();
    view.art_size = art_file_->data_size();
    view.palette = palette_.get();
    view.header = art_file_->header();
    view.tiles = art_file_->tiles(); // Copy the tiles vector (metadata only)

    return view;
}

// ImageView method implementations
bool ImageView::save_to_png(const std::filesystem::path& path, PngWriter::Options options) const {
    if (!parent || !parent->palette) {
        throw ArtException("Invalid ImageView state: parent or palette is null");
    }

    const uint8_t* pixels = pixel_data();
    if (!pixels) {
        // Empty tile - create an empty file or skip?
        // For now, skip empty tiles
        return true;
    }

    ArtFile::Tile tile = parent->get_tile(tile_index);
    return PngWriter::write_png(path, *parent->palette, tile, pixels, size(), options);
}

std::vector<uint8_t> ImageView::extract_to_png(PngWriter::Options options) const {
    if (!parent || !parent->palette) {
        throw ArtException("Invalid ImageView state: parent or palette is null");
    }

    const uint8_t* pixels = pixel_data();
    if (!pixels) {
        // Empty tile - return empty vector
        return {};
    }

    ArtFile::Tile tile = parent->get_tile(tile_index);
    std::vector<uint8_t> result;
    if (!PngWriter::write_png_to_memory(result, *parent->palette, tile, pixels, size(), options)) {
        throw ArtException("Failed to extract PNG to memory");
    }
    return result;
}

bool ImageView::save_to_tga(const std::filesystem::path& path) const {
    if (!parent || !parent->palette) {
        throw ArtException("Invalid ImageView state: parent or palette is null");
    }

    const uint8_t* pixels = pixel_data();
    if (!pixels) {
        // Empty tile - skip
        return true;
    }

    ArtFile::Tile tile = parent->get_tile(tile_index);
    return TgaWriter::write_tga(path, *parent->palette, tile, pixels, size());
}

std::vector<uint8_t> ImageView::extract_to_tga() const {
    if (!parent || !parent->palette) {
        throw ArtException("Invalid ImageView state: parent or palette is null");
    }

    const uint8_t* pixels = pixel_data();
    if (!pixels) {
        // Empty tile - return empty vector
        return {};
    }

    ArtFile::Tile tile = parent->get_tile(tile_index);
    std::vector<uint8_t> result;
    if (!TgaWriter::write_tga_to_memory(result, *parent->palette, tile, pixels, size())) {
        throw ArtException("Failed to extract TGA to memory");
    }
    return result;
}

} // namespace art2img