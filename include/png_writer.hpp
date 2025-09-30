#pragma once

#include "art_file.hpp"
#include "palette.hpp"
#include <vector>
#include <string>

namespace art2image {

class PngWriter {
public:
    struct Options {
        bool enable_alpha = true;           // Enable alpha channel support
        bool premultiply_alpha = true;      // Apply premultiplication for upscaling
        bool matte_hygiene = false;         // Apply alpha matte hygiene (erode + blur)
        
        Options() {}
    };
    
    static bool write_png(const std::string& filename,
                         const Palette& palette,
                         const ArtFile::Tile& tile,
                         const std::vector<uint8_t>& pixel_data,
                         const Options& options = Options());
    
    // Public for testing
    static bool is_magenta(uint8_t r, uint8_t g, uint8_t b);
    
private:
    static std::vector<uint8_t> convert_to_rgba(const Palette& palette,
                                               const ArtFile::Tile& tile,
                                               const std::vector<uint8_t>& pixel_data,
                                               const Options& options);
    
    static void apply_premultiplication(std::vector<uint8_t>& rgba_data);
    static void apply_matte_hygiene(std::vector<uint8_t>& rgba_data, int width, int height);
};

} // namespace art2image