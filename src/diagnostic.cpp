#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <filesystem>
#include <string>

namespace art2img {

uint32_t read_little_endian_uint32(std::ifstream& file) {
    uint32_t value = 0;
    file.read(reinterpret_cast<char*>(&value), sizeof(value));
    return value;
}

void analyze_art_file(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        std::cerr << "Error: Cannot open file " << filename << std::endl;
        return;
    }
    
    std::cout << "\nAnalyzing " << filename << std::endl;
    
    // Read header
    uint32_t version = read_little_endian_uint32(file);
    uint32_t num_tiles = read_little_endian_uint32(file);
    uint32_t start_tile = read_little_endian_uint32(file);
    uint32_t end_tile = read_little_endian_uint32(file);
    
    std::cout << "Header values (little-endian):" << std::endl;
    std::cout << "  Version:    " << version << " (0x" << std::hex << version << std::dec << ")" << std::endl;
    std::cout << "  Num tiles:  " << num_tiles << " (0x" << std::hex << num_tiles << std::dec << ")" << std::endl;
    std::cout << "  Start tile: " << start_tile << " (0x" << std::hex << start_tile << std::dec << ")" << std::endl;
    std::cout << "  End tile:   " << end_tile << " (0x" << std::hex << end_tile << std::dec << ")" << std::endl;
    
    // Calculate actual tile count
    uint32_t calculated_tiles = (end_tile >= start_tile) ? (end_tile - start_tile + 1) : 0;
    std::cout << "\nCalculated tile count: " << calculated_tiles << std::endl;
    
    // Check if this matches the expected Duke3D format
    std::cout << "\nFormat analysis:" << std::endl;
    std::cout << "  Version valid (==1): " << (version == 1 ? "YES" : "NO") << std::endl;
    std::cout << "  Tile range valid:    " << (end_tile >= start_tile ? "YES" : "NO") << std::endl;
    std::cout << "  Num tiles matches:   " << (num_tiles == calculated_tiles ? "YES" : "NO") << std::endl;
    
    if (end_tile < start_tile) {
        std::cout << "\n[WARN] end_tile (" << end_tile << ") < start_tile (" << start_tile << ")" << std::endl;
        std::cout << "   This suggests either:" << std::endl;
        std::cout << "   1. Different byte order (big-endian?)" << std::endl;
        std::cout << "   2. Different ART format version" << std::endl;
        std::cout << "   3. File corruption" << std::endl;
    }
    
    // Try big-endian interpretation
    file.seekg(0);
    uint32_t version_be = 0;
    file.read(reinterpret_cast<char*>(&version_be), sizeof(version_be));
    
    // Swap bytes for big-endian
    version_be = ((version_be >> 24) & 0xff) | ((version_be >> 8) & 0xff00) | 
                 ((version_be << 8) & 0xff0000) | ((version_be << 24) & 0xff000000);
    
    std::cout << "\nBig-endian interpretation:" << std::endl;
    std::cout << "  Version: " << version_be << " (0x" << std::hex << version_be << std::dec << ")" << std::endl;
}

void check_header(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return;
    
    uint32_t version = read_little_endian_uint32(file);
    uint32_t num_tiles = read_little_endian_uint32(file);
    uint32_t start_tile = read_little_endian_uint32(file);
    uint32_t end_tile = read_little_endian_uint32(file);
    
    uint32_t calculated_tiles = (end_tile >= start_tile) ? (end_tile - start_tile + 1) : 0;
    
    std::cout << std::setw(15) << std::filesystem::path(filename).filename().string() 
              << " | Version: " << std::setw(2) << version
              << " | Num tiles: " << std::setw(4) << num_tiles
              << " | Start: " << std::setw(4) << start_tile
              << " | End: " << std::setw(4) << end_tile
              << " | Calc: " << std::setw(4) << calculated_tiles
              << " | Match: " << (num_tiles == calculated_tiles ? "YES" : "NO")
              << std::endl;
}

void check_all_headers(const std::string& directory = "assets") {
    std::cout << "File            | Version | Num tiles | Start | End  | Calc | Match" << std::endl;
    std::cout << "----------------|---------|-----------|-------|------|------|------" << std::endl;
    
    for (int i = 0; i <= 19; i++) {
        std::string filename = directory + "/TILES" 
                             + std::string(3 - std::to_string(i).length(), '0') 
                             + std::to_string(i) + ".ART";
        
        if (std::filesystem::exists(filename)) {
            check_header(filename);
        }
    }
}

void show_usage(const std::string& program_name) {
    std::cout << "Usage: " << program_name << " [OPTIONS] [FILE]" << std::endl;
    std::cout << "\nOptions:" << std::endl;
    std::cout << "  -a, --analyze FILE    Analyze a specific ART file in detail" << std::endl;
    std::cout << "  -c, --check-all       Check headers of all TILES*.ART files in assets/" << std::endl;
    std::cout << "  -d, --directory DIR   Specify directory for --check-all (default: assets)" << std::endl;
    std::cout << "  -h, --help            Show this help message" << std::endl;
    std::cout << "\nExamples:" << std::endl;
    std::cout << "  " << program_name << " -a assets/TILES000.ART" << std::endl;
    std::cout << "  " << program_name << " -c" << std::endl;
    std::cout << "  " << program_name << " -c -d my_assets" << std::endl;
}

} // namespace art2img

int main(int argc, char* argv[]) {
    if (argc < 2) {
        art2img::show_usage(argv[0]);
        return 1;
    }
    
    std::string mode;
    std::string filename;
    std::string directory = "assets";
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "-a" || arg == "--analyze") {
            if (i + 1 < argc) {
                filename = argv[++i];
                mode = "analyze";
            } else {
                std::cerr << "Error: --analyze requires a filename" << std::endl;
                return 1;
            }
        } else if (arg == "-c" || arg == "--check-all") {
            mode = "check-all";
        } else if (arg == "-d" || arg == "--directory") {
            if (i + 1 < argc) {
                directory = argv[++i];
            } else {
                std::cerr << "Error: --directory requires a path" << std::endl;
                return 1;
            }
        } else if (arg == "-h" || arg == "--help") {
            art2img::show_usage(argv[0]);
            return 0;
        } else {
            // Assume it's a filename for backward compatibility
            filename = arg;
            mode = "analyze";
        }
    }
    
    if (mode == "analyze") {
        art2img::analyze_art_file(filename);
    } else if (mode == "check-all") {
        art2img::check_all_headers(directory);
    } else {
        art2img::show_usage(argv[0]);
        return 1;
    }
    
    return 0;
}