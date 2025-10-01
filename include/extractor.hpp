#pragma once

#include "art_file.hpp"
#include "palette.hpp"
#include "tga_writer.hpp"
#include "png_writer.hpp"
#include "threading.hpp"
#include <string>
#include <atomic>
#include <mutex>

namespace art2img {

class ArtExtractor {
public:
    enum class OutputFormat {
        TGA,
        PNG
    };
    
    struct Options {
        std::string output_dir = ".";
        unsigned num_threads = std::thread::hardware_concurrency();
        bool verbose = true;
        bool dump_animation = true;
        bool merge_animation_data = false; // Merge all animdata.ini files into one
        OutputFormat format = OutputFormat::PNG;
        PngWriter::Options png_options;
        
        bool is_valid() const {
            return !output_dir.empty() && num_threads > 0;
        }
    };
    
    struct Stats {
        std::atomic<uint32_t> tiles_processed{0};
        std::atomic<uint32_t> tiles_successful{0};
        std::atomic<uint32_t> tiles_failed{0};
        
        void reset() {
            tiles_processed = 0;
            tiles_successful = 0;
            tiles_failed = 0;
        }
    };
    
    ArtExtractor(ArtFile& art_file, Palette& palette);
    
    bool extract(const Options& options);
    
    // Accessors
    const Stats& stats() const { return stats_; }
    const ArtFile& art_file() const { return art_file_; }
    const Palette& palette() const { return palette_; }
    
private:
    bool extract_tile(uint32_t index, const std::string& output_dir);
    bool dump_animation_data(const std::string& output_dir);
    void update_progress();
    
    ArtFile& art_file_;
    Palette& palette_;
    Stats stats_;
    Options options_;
    std::mutex file_mutex_; // For thread-safe file access
};

} // namespace art2img