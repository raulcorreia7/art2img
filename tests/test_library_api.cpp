#include "extractor_api.hpp"
#include <iostream>
#include <fstream>
#include <vector>

int main() {
    std::cout << "Testing art2img library API..." << std::endl;
    
    // Create extractor API instance
    art2img::ExtractorAPI extractor;
    
    // Load ART file from memory
    std::ifstream art_file("../tests/assets/TILES000.ART", std::ios::binary);
    if (!art_file) {
        std::cerr << "Error: Cannot open TILES000.ART" << std::endl;
        return 1;
    }
    
    // Read file into memory
    art_file.seekg(0, std::ios::end);
    size_t art_size = art_file.tellg();
    art_file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> art_data(art_size);
    art_file.read(reinterpret_cast<char*>(art_data.data()), art_size);
    art_file.close();
    
    // Load ART data from memory
    if (!extractor.load_art_from_memory(art_data.data(), art_size)) {
        std::cerr << "Error: Failed to load ART from memory" << std::endl;
        return 1;
    }
    
    std::cout << "Loaded ART file with " << extractor.get_tile_count() << " tiles" << std::endl;
    
    // Load palette from memory
    std::ifstream palette_file("../tests/assets/PALETTE.DAT", std::ios::binary);
    if (!palette_file) {
        std::cerr << "Error: Cannot open PALETTE.DAT" << std::endl;
        return 1;
    }
    
    // Read palette into memory
    palette_file.seekg(0, std::ios::end);
    size_t palette_size = palette_file.tellg();
    palette_file.seekg(0, std::ios::beg);
    
    std::vector<uint8_t> palette_data(palette_size);
    palette_file.read(reinterpret_cast<char*>(palette_data.data()), palette_size);
    palette_file.close();
    
    // Load palette data from memory
    if (!extractor.load_palette_from_memory(palette_data.data(), palette_size)) {
        std::cerr << "Error: Failed to load palette from memory" << std::endl;
        return 1;
    }
    
    std::cout << "Loaded palette from memory" << std::endl;
    
    // Extract a single tile to PNG
    art2img::ExtractionResult result = extractor.extract_tile(0);
    if (!result.success) {
        std::cerr << "Error: Failed to extract tile: " << result.error_message << std::endl;
        return 1;
    }
    
    std::cout << "Extracted tile 0: " << result.width << "x" << result.height 
              << " pixels, " << result.image_data.size() << " bytes" << std::endl;
    
    // Save to file
    std::ofstream output_file("../tests/output/tile0000.png", std::ios::binary);
    if (!output_file) {
        std::cerr << "Error: Cannot create output file" << std::endl;
        return 1;
    }
    
    output_file.write(reinterpret_cast<const char*>(result.image_data.data()), result.image_data.size());
    output_file.close();
    
    std::cout << "Saved tile0000.png to output directory" << std::endl;
    
    // Test batch extraction
    std::vector<art2img::ExtractionResult> results = extractor.extract_all_tiles();
    std::cout << "Extracted " << results.size() << " tiles in batch" << std::endl;
    
    // Count successful extractions
    size_t success_count = 0;
    for (const auto& res : results) {
        if (res.success) {
            success_count++;
        }
    }
    
    std::cout << "Successfully extracted " << success_count << " tiles" << std::endl;
    
    std::cout << "Library API test completed successfully!" << std::endl;
    return 0;
}