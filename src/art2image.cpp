#include "cli.hpp"
#include "art_file.hpp"
#include "palette.hpp"
#include "extractor.hpp"
#include <iostream>
#include <filesystem>

namespace {

// Resolve palette file path using dynamic resolution
std::string resolve_palette_path(const std::string& user_path, const std::string& art_file_path) {
    // 1. User-specified path (highest priority)
    if (!user_path.empty()) {
        if (std::filesystem::exists(user_path)) {
            return user_path;
        }
        // If user specified but file doesn't exist, we'll try other locations
    }
    
    // 2. Same directory as ART file
    std::filesystem::path art_dir = std::filesystem::path(art_file_path).parent_path();
    std::string art_dir_palette = (art_dir / "palette.dat").string();
    if (std::filesystem::exists(art_dir_palette)) {
        return art_dir_palette;
    }
    
    // 3. Current working directory (current behavior)
    if (std::filesystem::exists("palette.dat")) {
        return "palette.dat";
    }
    
    // 4. Assets directory (project-specific)
    if (std::filesystem::exists("assets/palette.dat")) {
        return "assets/palette.dat";
    }
    
    // 5. Duke3D pipeline paths (convenience for Duke3D upscaling)
    if (std::filesystem::exists("../../../../build/duke3d/PALETTE.DAT")) {
        return "../../../../build/duke3d/PALETTE.DAT";
    }
    
    // 6. Return empty string (trigger fallback to Blood palette)
    return "";
}

} // anonymous namespace

// Function to append animation data to merged file (for directory mode with merge enabled)
bool append_animation_data_to_merged_file(const art2image::ArtFile& art_file, const std::string& output_dir) {
    const char* animation_types[] = {"none", "oscillation", "forward", "backward"};
    
    std::string filename = output_dir + "/animdata.ini";
    std::ofstream file(filename, std::ios::app); // Append mode
    
    if (!file.is_open()) {
        return false;
    }
    
    // Write header for this ART file
    file << "; Animation data from \"" << art_file.filename() << "\"\n"
                               << "; Extracted by art2image\n"
         << "\n";
    
    const auto& tiles = art_file.tiles();
    const auto& header = art_file.header();
    
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
    
    return true;
}

// Function to process a single ART file
bool process_single_file(const art2image::CLI::Options& options, const std::string& art_file_path, const std::string& output_subdir = "", bool is_directory_mode = false) {
    try {
        if (options.extractor_options.verbose) {
            std::cout << "Processing ART file: " << art_file_path << std::endl;
        }
        
        // Load ART file
        art2image::ArtFile art_file(art_file_path);
        
        // Load palette with dynamic path resolution
        art2image::Palette palette;
        std::string palette_path = resolve_palette_path(options.palette_file, art_file_path);
        
        if (!palette_path.empty() && palette.load_from_file(palette_path)) {
            if (options.extractor_options.verbose) {
                std::cout << "Using palette file: " << palette_path << std::endl;
            }
        } else {
            if (options.extractor_options.verbose) {
                if (!palette_path.empty()) {
                    std::cout << "Warning: Cannot open palette file '" << palette_path << "'" << std::endl;
                }
                std::cout << "Using default Duke Nukem 3D palette" << std::endl;
            }
            // Use Duke Nukem 3D as default palette instead of Blood
            palette.load_duke3d_default();
        }
        
        // Create extractor with modified output directory for directory processing
        art2image::ArtExtractor::Options extractor_options = options.extractor_options;
        if (!output_subdir.empty()) {
            extractor_options.output_dir = (std::filesystem::path(extractor_options.output_dir) / output_subdir).string();
        }
        
        // In directory mode with merge enabled, don't create individual animdata.ini files
        if (is_directory_mode && options.extractor_options.merge_animation_data) {
            extractor_options.dump_animation = false;
        }
        
        // Create extractor and perform extraction
        art2image::ArtExtractor extractor(art_file, palette);
        bool extraction_success = extractor.extract(extractor_options);
        
        // If in directory mode with merge enabled, append animation data to merged file
        if (is_directory_mode && options.extractor_options.merge_animation_data && extraction_success) {
            if (!append_animation_data_to_merged_file(art_file, options.extractor_options.output_dir)) {
                std::cerr << "Warning: Failed to append animation data from " << art_file_path << " to merged file" << std::endl;
            }
        }
        
        return extraction_success;
        
    } catch (const std::exception& e) {
        std::cerr << "Error processing " << art_file_path << ": " << e.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Error: Unknown exception occurred while processing " << art_file_path << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        auto options = art2image::CLI::parse_arguments(argc, argv);
        
        if (options.show_help) {
            return 0;
        }
        
        if (options.extractor_options.verbose) {
            std::cout << "art2image - Multi-threaded ART to image converter (TGA/PNG)" << std::endl;
            std::cout << "==========================================================" << std::endl;
            std::cout << std::endl;
        }
        
        bool success = true;
        
        if (options.process_directory) {
            // Process all ART files in directory
            if (options.extractor_options.verbose) {
                std::cout << "Processing ART files in directory: " << options.art_directory << std::endl;
            }
            
            // If merge animation data is enabled, create/clear the merged file first
            if (options.extractor_options.merge_animation_data) {
                std::string merged_ini_path = (std::filesystem::path(options.extractor_options.output_dir) / "animdata.ini").string();
                std::ofstream merged_file(merged_ini_path);
                if (merged_file.is_open()) {
                    merged_file << "; Merged animation data from all ART files\n"
         << "; Extracted by art2image\n"
                               << "; Generated: " << __DATE__ << " " << __TIME__ << "\n"
                               << "\n";
                    if (options.extractor_options.verbose) {
                        std::cout << "Created merged animation data file: " << merged_ini_path << std::endl;
                    }
                } else {
                    std::cerr << "Warning: Failed to create merged animation data file" << std::endl;
                }
            }
            
            int processed_files = 0;
            int successful_files = 0;
            
            for (const auto& entry : std::filesystem::directory_iterator(options.art_directory)) {
                if (entry.is_regular_file()) {
                    std::string extension = entry.path().extension().string();
                    if (extension == ".art" || extension == ".ART") {
                        processed_files++;
                        // Use filename without extension as subdirectory
                        std::string subdir = entry.path().stem().string();
                        if (process_single_file(options, entry.path().string(), subdir, true)) {
                            successful_files++;
                        } else {
                            success = false;
                        }
                    }
                }
            }
            
            if (options.extractor_options.verbose) {
                std::cout << "Directory processing complete: " << successful_files 
                          << "/" << processed_files << " files successful" << std::endl;
            }
            
        } else {
            // Process single file
            success = process_single_file(options, options.art_file, "", false);
        }
        
        return success ? 0 : 1;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown exception occurred" << std::endl;
        return 1;
    }
}