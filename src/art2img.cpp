#include "art_file.hpp"
#include "palette.hpp"
#include "extractor_api.hpp"
#include "exceptions.hpp"
#include "version.hpp"
#include <CLI/CLI.hpp>
#include <iostream>
#include <filesystem>
#include <iomanip>
#include <sstream>
#include <thread>
#include <vector>

namespace {

struct PaletteResolutionResult {
    std::string resolved_path;
    std::vector<std::string> candidates;
    bool user_hint_missing = false;

    bool has_resolution() const { return !resolved_path.empty(); }
};

PaletteResolutionResult resolve_palette_path(const std::string& user_path, const std::string& art_file_path) {
    PaletteResolutionResult result;

    auto try_candidate = [&](const std::filesystem::path& candidate, bool mark_user = false) -> bool {
        if (candidate.empty()) {
            return false;
        }

        std::string candidate_str = candidate.string();
        result.candidates.push_back(candidate_str);

        std::error_code ec;
        if (std::filesystem::exists(candidate, ec)) {
            result.resolved_path = candidate_str;
            return true;
        }

        if (mark_user) {
            result.user_hint_missing = true;
        }
        return false;
    };

    if (!user_path.empty() && try_candidate(user_path, true)) {
        return result;
    }

    const std::filesystem::path art_dir = std::filesystem::path(art_file_path).parent_path();
    if (!art_dir.empty()) {
        if (try_candidate(art_dir / "palette.dat")) {
            return result;
        }
        if (try_candidate(art_dir / "PALETTE.DAT")) {
            return result;
        }
    }

    if (try_candidate("palette.dat")) {
        return result;
    }
    if (try_candidate("PALETTE.DAT")) {
        return result;
    }
    if (try_candidate("assets/palette.dat")) {
        return result;
    }
    if (try_candidate("assets/PALETTE.DAT")) {
        return result;
    }

    return result;
}

} // anonymous namespace

// Function to append animation data to merged file (for directory mode with merge enabled)
bool append_animation_data_to_merged_file(const art2img::ArtFile& art_file, const std::string& output_dir) {
    const char* animation_types[] = {"none", "oscillation", "forward", "backward"};
    
    std::string filename = output_dir + "/animdata.ini";
    std::ofstream file(filename, std::ios::app); // Append mode
    
    if (!file.is_open()) {
        return false;
    }
    
    // Write header for this ART file
    file << "; Animation data from \"" << art_file.filename() << "\"\n"
<< "; Extracted by art2img\n"
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

// Options structure for process_single_file
struct ProcessOptions {
    std::string palette_file;
    std::string output_dir;
    unsigned num_threads = 0;
    std::string format = "png";
    bool enable_magenta_transparency = true;
    bool verbose = false;
    bool dump_animation = true;
    bool merge_animation_data = false;
    mutable bool palette_warning_emitted = false;
};

// Function to process a single ART file
bool process_single_file(const ProcessOptions& options, const std::string& art_file_path, const std::string& output_subdir = "", bool is_directory_mode = false) {
    try {
        if (options.verbose) {
            std::cout << "Processing ART file: " << art_file_path << std::endl;
        }
        
        // Load ART file
        art2img::ArtFile art_file(art_file_path);
        
        // Load palette with dynamic path resolution
        art2img::Palette palette;
        PaletteResolutionResult palette_resolution = resolve_palette_path(options.palette_file, art_file_path);
        const std::string& palette_path = palette_resolution.resolved_path;
        bool palette_loaded = false;

        if (!palette_path.empty()) {
            try {
                palette_loaded = palette.load_from_file(palette_path);
            } catch (const art2img::ArtException& e) {
                if (options.verbose) {
                    std::cout << "Warning: " << e.what() << std::endl;
                }
            }
        }

        if (palette_loaded) {
            if (options.verbose) {
                std::cout << "Using palette file: " << palette_path << std::endl;
            }
        } else {
            if (options.verbose) {
                if (!palette_path.empty()) {
                    std::cout << "Warning: Cannot open palette file '" << palette_path << "'" << std::endl;
                } else if (palette_resolution.user_hint_missing && !options.palette_file.empty()) {
                    std::cout << "Warning: Cannot locate palette file '" << options.palette_file << "'" << std::endl;
                } else {
                    std::cout << "Warning: No palette file detected" << std::endl;
                }
                std::cout << "Using default Duke Nukem 3D palette" << std::endl;
            }

            if (!options.palette_warning_emitted) {
                std::ostringstream warn;
                if (palette_resolution.user_hint_missing && !options.palette_file.empty()) {
                    warn << "Warning: Palette file '" << options.palette_file << "' not found.";
                } else if (!palette_path.empty()) {
                    warn << "Warning: Failed to load palette file '" << palette_path << "'.";
                } else {
                    warn << "Warning: Could not locate palette file.";
                }

                if (!palette_resolution.candidates.empty()) {
                    warn << " Checked: ";
                    for (size_t i = 0; i < palette_resolution.candidates.size(); ++i) {
                        if (i > 0) {
                            warn << ", ";
                        }
                        warn << palette_resolution.candidates[i];
                    }
                    warn << '.';
                }

                warn << " Using built-in Duke Nukem 3D palette.";
                std::cerr << warn.str() << std::endl;
                options.palette_warning_emitted = true;
            }

            palette.load_duke3d_default();
        }

        // Create output directory path for directory processing
        std::string final_output_dir = options.output_dir;
        if (!output_subdir.empty()) {
            final_output_dir = (std::filesystem::path(final_output_dir) / output_subdir).string();
        }

        // Ensure output directory exists before writing files
        std::error_code dir_error;
        if (!final_output_dir.empty()) {
            std::filesystem::create_directories(final_output_dir, dir_error);
        }

        if (dir_error) {
            std::cerr << "Error: Failed to create output directory '" << final_output_dir
                      << "': " << dir_error.message() << std::endl;
            return false;
        }

        // Load ArtFile for animation data processing
        art2img::ArtFile art_file_obj(art_file_path);
        if (!art_file_obj.is_open()) {
            std::cerr << "Error: Failed to open ART file: " << art_file_path << std::endl;
            return false;
        }

        // Read ART file into memory for zero-copy processing
        std::ifstream art_file_stream(art_file_path, std::ios::binary | std::ios::ate);
        if (!art_file_stream.is_open()) {
            std::cerr << "Error: Cannot open ART file: " << art_file_path << std::endl;
            return false;
        }

        std::streamsize art_file_size = art_file_stream.tellg();
        art_file_stream.seekg(0, std::ios::beg);

        std::vector<uint8_t> art_data(art_file_size);
        if (!art_file_stream.read(reinterpret_cast<char*>(art_data.data()), art_file_size)) {
            std::cerr << "Error: Cannot read ART file data: " << art_file_path << std::endl;
            return false;
        }

        // Create ExtractorAPI and load everything from memory
        art2img::ExtractorAPI extractor;
        extractor.load_art_from_memory(art_data.data(), art_data.size());
        extractor.load_palette_from_memory(palette.raw_data().data(), palette.raw_data().size());
        art2img::ArtView art_view = extractor.get_art_view();

        // Process tiles sequentially for ordered CLI output
        bool extraction_success = true;

        for (uint32_t i = 0; i < art_view.image_count(); ++i) {
            try {
                art2img::ImageView image_view{&art_view, i};
                if (image_view.pixel_data() == nullptr || image_view.width() == 0 || image_view.height() == 0) {
                    continue; // Skip empty tiles
                }

                // Write the tile to disk
                std::string tile_filename = final_output_dir + "/tile" + std::to_string(i + art_view.header.start_tile) + "." + options.format;

                bool success = false;
                if (options.format == "tga") {
                    success = image_view.save_to_tga(tile_filename);
                } else {
                    art2img::PngWriter::Options png_options;
                    png_options.enable_magenta_transparency = options.enable_magenta_transparency;
                    success = image_view.save_to_png(tile_filename, png_options);
                }

                if (!success) {
                    extraction_success = false;
                    std::cerr << "Warning: Failed to write tile " << i + art_view.header.start_tile << std::endl;
                }
            } catch (const std::exception& e) {
                std::cerr << "Error processing tile " << i + art_view.header.start_tile << ": " << e.what() << std::endl;
                extraction_success = false;
            }
        }

        // Handle animation data output
        if ((options.dump_animation && !is_directory_mode) || (options.merge_animation_data && is_directory_mode)) {
            if (!append_animation_data_to_merged_file(art_file_obj, options.output_dir)) {
                if (!is_directory_mode) {
                    std::cerr << "Warning: Failed to write animation data for " << art_file_path << std::endl;
                }
            }
        }
        
        return extraction_success;
        
    } catch (const art2img::ArtException& e) {
        std::cerr << "Error processing " << art_file_path << ": " << e.what() << std::endl;
        return false;
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
// CLI11 options
        std::string output_dir = ".";
        int num_threads = std::thread::hardware_concurrency();
        std::string palette_file = "";
        std::string format = "png";
        bool fix_transparency = true;
        bool quiet = false;
        bool no_anim = false;
        bool merge_anim = false;
        std::string input_path;
        
        CLI::App app{"Extract pictures from ART files to TGA or PNG format"};
        app.set_version_flag("-v,--version", "art2img " ART2IMG_VERSION);
        
        // Positional argument
        app.add_option("ART_FILE|ART_DIRECTORY", input_path, "Input ART file or directory")
            ->required();
        
        // Options
        app.add_option("-o,--output", output_dir, "Output directory")
            ->default_val(".");
        app.add_option("-t,--threads", num_threads, "Number of threads (-1 for all cores)")
            ->default_val(static_cast<int>(std::thread::hardware_concurrency()));
        app.add_option("-p,--palette", palette_file,
                       "Palette file path (defaults to built-in Duke Nukem 3D palette)");
        app.add_option("-f,--format", format, "Output format: tga or png")
            ->default_val("png")
            ->check(CLI::IsMember({"tga", "png"}));
        auto* fix_flag = app.add_flag("-F,--fix-transparency", "Enable magenta transparency fix");
        auto* no_fix_flag = app.add_flag("-N,--no-fix-transparency", "Disable magenta transparency fix");
        app.add_flag("-q,--quiet", quiet, "Suppress verbose output");
        app.add_flag("-n,--no-anim", no_anim, "Don't generate animdata.ini");
        app.add_flag("-m,--merge-anim", merge_anim, "Merge animation data into single file (directory mode)");
        
        // Configure conflicting flags
        fix_flag->excludes(no_fix_flag);
        no_fix_flag->excludes(fix_flag);
        
        CLI11_PARSE(app, argc, argv);
        
        // Handle -1 as special value for all threads
        if (num_threads == -1) {
            num_threads = static_cast<int>(std::thread::hardware_concurrency());
        }
        if (num_threads <= 0) {
            num_threads = static_cast<int>(std::thread::hardware_concurrency());
        }
        
        if (!quiet) {
            std::cout << "art2img - Multi-threaded ART to image converter (TGA/PNG)" << std::endl;
            std::cout << "==========================================================" << std::endl;
            std::cout << std::endl;
        }
        
        bool success = true;
        bool process_directory = std::filesystem::is_directory(input_path);
        
        if (process_directory) {
            // Process all ART files in directory
            if (!quiet) {
                std::cout << "Processing ART files in directory: " << input_path << std::endl;
            }

            // If merge animation data is enabled, create/clear the merged file first
            if (merge_anim) {
                std::string merged_ini_path = (std::filesystem::path(output_dir) / "animdata.ini").string();
                std::ofstream merged_file(merged_ini_path);
                if (merged_file.is_open()) {
                    merged_file << "; Merged animation data from all ART files\n"
         << "; Extracted by art2img\n"
                               << "; Generated: " << __DATE__ << " " << __TIME__ << "\n"
                               << "\n";
                    if (!quiet) {
                        std::cout << "Created merged animation data file: " << merged_ini_path << std::endl;
                    }
                } else {
                    std::cerr << "Warning: Failed to create merged animation data file" << std::endl;
                }
            }

            int processed_files = 0;
            int successful_files = 0;

            // Create options structure for process_single_file
            ProcessOptions options;
            options.palette_file = palette_file;
            options.output_dir = output_dir;
            options.num_threads = static_cast<unsigned>(num_threads);
            options.format = format;
            options.enable_magenta_transparency = fix_transparency && (no_fix_flag->count() == 0);
            options.verbose = !quiet;
            options.dump_animation = !no_anim;
            options.merge_animation_data = merge_anim;
            
            for (const auto& entry : std::filesystem::directory_iterator(input_path)) {
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
            
            if (!quiet) {
                std::cout << "Directory processing complete: " << successful_files
                          << "/" << processed_files << " files successful" << std::endl;
            }

        } else {
            // Process single file
            ProcessOptions single_options;
            single_options.palette_file = palette_file;
            single_options.output_dir = output_dir;
            single_options.num_threads = static_cast<unsigned>(num_threads);
            single_options.format = format;
            single_options.enable_magenta_transparency = fix_transparency && (no_fix_flag->count() == 0);
            single_options.verbose = !quiet;
            single_options.dump_animation = !no_anim;
            single_options.merge_animation_data = merge_anim;

            success = process_single_file(single_options, input_path, "", false);
        }
        
        return success ? 0 : 1;
        
    } catch (const art2img::ArtException& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Error: Unknown exception occurred" << std::endl;
        return 1;
    }
}
