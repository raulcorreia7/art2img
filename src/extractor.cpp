#include "extractor.hpp"
#include <iostream>
#include <iomanip>
#include <sstream>
#include <filesystem>

namespace art2image {

ArtExtractor::ArtExtractor(ArtFile& art_file, Palette& palette)
    : art_file_(art_file), palette_(palette) {
}

bool ArtExtractor::extract(const Options& options) {
    if (!options.is_valid()) {
        std::cerr << "Error: Invalid extraction options" << std::endl;
        return false;
    }
    
    if (!art_file_.is_open()) {
        std::cerr << "Error: ART file is not open" << std::endl;
        return false;
    }
    
    if (!palette_.is_loaded()) {
        std::cerr << "Error: Palette is not loaded" << std::endl;
        return false;
    }
    
    options_ = options;
    stats_.reset();
    
    // Create output directory (remove existing content first)
    if (std::filesystem::exists(options_.output_dir)) {
        try {
            std::filesystem::remove_all(options_.output_dir);
        } catch (const std::filesystem::filesystem_error& e) {
            // If remove_all fails, try to remove files individually
            for (const auto& entry : std::filesystem::directory_iterator(options_.output_dir)) {
                std::filesystem::remove_all(entry.path());
            }
        }
    }
    std::filesystem::create_directories(options_.output_dir);
    
    if (options_.verbose) {
        std::cout << "Extracting " << art_file_.tiles().size() 
                  << " tiles using " << options_.num_threads << " threads..." << std::endl;
    }
    
    // Create thread pool
    ThreadPool pool(options_.num_threads);
    
    // Enqueue all tile extraction tasks
    const auto& tiles = art_file_.tiles();
    for (uint32_t i = 0; i < tiles.size(); ++i) {
        pool.enqueue([this, i]() {
            if (this->extract_tile(i, this->options_.output_dir)) {
                ++stats_.tiles_successful;
            } else {
                ++stats_.tiles_failed;
            }
            ++stats_.tiles_processed;
            
            if (options_.verbose && stats_.tiles_processed % 100 == 0) {
                update_progress();
            }
        });
    }
    
    // Wait for all tasks to complete
    pool.wait_all();
    
    if (options_.verbose) {
        update_progress();
        std::cout << std::endl;
    }
    
    // Dump animation data if requested
    if (options_.dump_animation) {
        if (!dump_animation_data(options_.output_dir)) {
            std::cerr << "Warning: Failed to create animation data file" << std::endl;
        }
    }
    
    if (options_.verbose) {
        std::cout << "Extraction complete: " << stats_.tiles_successful 
                  << " successful, " << stats_.tiles_failed << " failed" << std::endl;
    }
    
    return stats_.tiles_failed == 0;
}

bool ArtExtractor::extract_tile(uint32_t index, const std::string& output_dir) {
    const auto& tiles = art_file_.tiles();
    if (index >= tiles.size()) {
        return false;
    }
    
    const auto& tile = tiles[index];
    
    // Skip empty tiles
    if (tile.is_empty()) {
        return true;
    }
    
    // Generate filename with appropriate extension
    std::ostringstream filename;
    filename << output_dir << "/tile" 
             << std::setw(4) << std::setfill('0') 
             << (index + art_file_.header().start_tile);
    
    // Read tile data (thread-safe)
    std::vector<uint8_t> pixel_data;
    {
        std::lock_guard<std::mutex> lock(file_mutex_);
        if (!art_file_.read_tile_data(index, pixel_data)) {
            return false;
        }
    }
    
    // Write file based on format
    switch (options_.format) {
        case OutputFormat::PNG:
            filename << ".png";
            return PngWriter::write_png(filename.str(), palette_, tile, pixel_data, options_.png_options);
        
        case OutputFormat::TGA:
        default:
            filename << ".tga";
            return TgaWriter::write_tga(filename.str(), palette_, tile, pixel_data);
    }
}

bool ArtExtractor::dump_animation_data(const std::string& output_dir) {
    const char* animation_types[] = {"none", "oscillation", "forward", "backward"};
    
    std::string filename = output_dir + "/animdata.ini";
    std::ofstream file(filename);
    
    if (!file.is_open()) {
        return false;
    }
    
    if (options_.verbose) {
        std::cout << "Creating animation data file..." << std::endl;
    }
    
    file << "; This file contains animation data from \"" << art_file_.filename() << "\"\n"
         << "; Extracted by Art2Tga Optimized\n"
         << "\n";
    
    const auto& tiles = art_file_.tiles();
    const auto& header = art_file_.header();
    
    for (uint32_t i = 0; i < tiles.size(); ++i) {
        const auto& tile = tiles[i];
        
        if (tile.anim_data != 0) {
            // Check if tile has meaningful animation data
            if (tile.anim_frames() != 0 || tile.anim_type() != 0 || tile.anim_speed() != 0) {
                file << "[tile" << std::setw(4) << std::setfill('0') 
                     << (i + header.start_tile) << ".tga -> tile" 
                     << std::setw(4) << std::setfill('0')
                     << (i + header.start_tile + tile.anim_frames()) << ".tga]\n";
                file << "   AnimationType=" << animation_types[tile.anim_type()] << "\n";
                file << "   AnimationSpeed=" << tile.anim_speed() << "\n";
                file << "\n";
            }
            
            file << "[tile" << std::setw(4) << std::setfill('0') 
                 << (i + header.start_tile) << ".tga]\n";
            file << "   XCenterOffset=" << static_cast<int>(tile.x_offset()) << "\n";
            file << "   YCenterOffset=" << static_cast<int>(tile.y_offset()) << "\n";
            file << "   OtherFlags=" << tile.other_flags() << "\n";
            file << "\n";
        }
    }
    
    if (options_.verbose) {
        std::cout << " done" << std::endl;
    }
    
    return true;
}

void ArtExtractor::update_progress() {
    uint32_t processed = stats_.tiles_processed;
    uint32_t total = static_cast<uint32_t>(art_file_.tiles().size());
    
    std::cout << "\rExtracting: " << processed << "/" << total 
              << " (" << (processed * 100 / total) << "%)" << std::flush;
}

} // namespace art2image