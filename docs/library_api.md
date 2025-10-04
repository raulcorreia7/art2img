# art2img Library API

The art2img library provides a C++ API for extracting Duke Nukem 3D and Blood ART files to PNG/TGA images with full alpha channel support.

## Building the Library

```bash
# Build static and shared libraries
make library

# This will create:
# lib/libart2img.a (static library)
# lib/libart2img.so (shared library)
```

## API Usage

### Basic Usage

```cpp
#include "extractor_api.hpp"

// Create extractor instance
art2img::ExtractorAPI extractor;

// Load ART file from memory
std::vector<uint8_t> art_data = load_file_data("TILES000.ART");
extractor.load_art_from_memory(art_data.data(), art_data.size());

// Load palette from memory
std::vector<uint8_t> palette_data = load_file_data("PALETTE.DAT");
extractor.load_palette_from_memory(palette_data.data(), palette_data.size());

// Extract a single tile
art2img::ExtractionResult result = extractor.extract_tile(0);

if (result.success) {
    // Save PNG data to file
    save_to_file("tile0000.png", result.image_data);
}
```

### Key Classes

#### ExtractorAPI

Main interface for extracting ART files.

```cpp
class ExtractorAPI {
public:
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
    ExtractionResult extract_tile(uint32_t tile_index, ImageFormat format,
                                  ImageWriter::Options options = ImageWriter::Options());
    ExtractionResult extract_tile_png(uint32_t tile_index,
                                      ImageWriter::Options options = ImageWriter::Options());
    ExtractionResult extract_tile_tga(uint32_t tile_index,
                                      ImageWriter::Options options = ImageWriter::Options());
    
    // Batch extraction
    std::vector<ExtractionResult> extract_all_tiles(ImageFormat format,
                                                     ImageWriter::Options options = ImageWriter::Options());
    std::vector<ExtractionResult> extract_all_tiles_png(ImageWriter::Options options = ImageWriter::Options());
    std::vector<ExtractionResult> extract_all_tiles_tga(ImageWriter::Options options = ImageWriter::Options());
};
```

// Convenience Functions
//
// The API provides convenience wrapper functions that call the generic functions
// with appropriate format parameters:
// - extract_tile_png() calls extract_tile() with ImageFormat::PNG
// - extract_tile_tga() calls extract_tile() with ImageFormat::TGA
// - extract_all_tiles_png() calls extract_all_tiles() with ImageFormat::PNG
// - extract_all_tiles_tga() calls extract_all_tiles() with ImageFormat::TGA
//
// These functions maintain backward compatibility while providing a cleaner API
// that follows the facade pattern - they are simple wrappers around the generic
// functions but provide a more convenient interface for common use cases.

#### ExtractionResult

Contains the result of an extraction operation.

```cpp
struct ExtractionResult {
    bool success;              // Whether extraction was successful
    std::string error_message; // Error message if unsuccessful
    std::vector<uint8_t> image_data; // Image data in PNG or TGA format
    std::string format;        // "png" or "tga"
    uint32_t tile_index;       // Tile index
    uint16_t width;            // Image width
    uint16_t height;           // Image height
    
    // Animation data
    uint32_t anim_frames;
    uint32_t anim_type;
    int8_t x_offset;
    int8_t y_offset;
    uint32_t anim_speed;
    uint32_t other_flags;
};
```

#### ImageWriter::Options

Options for image extraction.

```cpp
struct ImageWriter::Options {
    bool enable_alpha = true;        // Enable alpha channel support (PNG only)
    bool premultiply_alpha = false;  // Apply premultiplication for upscaling (PNG only)
    bool matte_hygiene = false;      // Apply alpha matte hygiene (erode + blur) (PNG only)
    bool fix_transparency = true;    // Enable magenta transparency processing (PNG only)
};
```

## Example Usage

### Extract Single Tile

```cpp
#include "extractor_api.hpp"
#include <iostream>
#include <fstream>

int main() {
    art2img::ExtractorAPI extractor;
    
    // Load files
    extractor.load_art_file("TILES000.ART");
    extractor.load_palette_file("PALETTE.DAT");
    
    // Extract tile 0
    art2img::ExtractionResult result = extractor.extract_tile(0, art2img::ImageFormat::PNG);
    
    if (result.success) {
        std::ofstream file("tile0000.png", std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()), 
                   result.image_data.size());
        std::cout << "Extracted tile: " << result.width << "x" << result.height << std::endl;
    } else {
        std::cerr << "Error: " << result.error_message << std::endl;
    }
    
    return 0;
}
```

### Batch Extraction

```cpp
// Extract all tiles
std::vector<art2img::ExtractionResult> results = extractor.extract_all_tiles(art2img::ImageFormat::PNG);

for (const auto& result : results) {
    if (result.success) {
        std::string filename = "tile" + std::to_string(result.tile_index) + ".png";
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(result.image_data.data()), 
                   result.image_data.size());
    }
}
```

## Linking with the Library

### Using the Static Library

```bash
g++ -std=c++20 -Iinclude your_code.cpp -Llib -lart2img -lpthread -o your_program
```

### Using the Shared Library

```bash
g++ -std=c++20 -Iinclude your_code.cpp -Llib -lart2img -lpthread -o your_program
LD_LIBRARY_PATH=lib ./your_program
```

## Key Features

- **Memory-based operations**: Load ART files and palettes from memory buffers
- **PNG/TGA output**: Extract to either PNG or TGA format
- **Alpha channel support**: Automatic alpha channel from magenta keying
- **Thread-safe**: Designed for multi-threaded applications
- **Cross-platform**: Works on Linux x86_64 and Windows x86_64
- **Backward compatible**: Maintains existing file-based CLI functionality
