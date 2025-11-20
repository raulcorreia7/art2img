#include <art2img/art.hpp>
#include <fstream>
#include <vector>
#include <span>
#include <cstring>

namespace art2img {

namespace {

// Helper to read integers from span
uint32_t read_u32(std::span<const Byte> data, size_t offset) {
    uint32_t val;
    std::memcpy(&val, data.data() + offset, sizeof(val));
    return val;
}

uint16_t read_u16(std::span<const Byte> data, size_t offset) {
    uint16_t val;
    std::memcpy(&val, data.data() + offset, sizeof(val));
    return val;
}

} // namespace

Result<ArtFile> load_art(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return std::unexpected(Error{"Failed to open file: " + path.string()});
    }

    auto size = file.tellg();
    if (size < 0) {
         return std::unexpected(Error{"Failed to get file size"});
    }
    
    std::vector<Byte> buffer(static_cast<size_t>(size));
    file.seekg(0, std::ios::beg);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        return std::unexpected(Error{"Failed to read file content"});
    }

    std::span<const Byte> data(buffer);
    size_t offset = 0;

    // Header is 16 bytes
    if (data.size() < 16) {
        return std::unexpected(Error{"File too small for header"});
    }

    uint32_t version = read_u32(data, offset); offset += 4;
    if (version != 1) {
        return std::unexpected(Error{"Invalid version, expected 1"});
    }

    // Skip num_tiles (4 bytes)
    offset += 4;
    
    uint32_t tile_start = read_u32(data, offset); offset += 4;
    uint32_t tile_end = read_u32(data, offset); offset += 4;

    if (tile_end < tile_start) {
        return std::unexpected(Error{"Invalid tile range"});
    }

    size_t tile_count = tile_end - tile_start + 1;

    // Arrays:
    // Widths (short) * tile_count
    // Heights (short) * tile_count
    // Picanm (int) * tile_count
    
    size_t widths_size = tile_count * 2;
    size_t heights_size = tile_count * 2;
    size_t picanm_size = tile_count * 4;
    
    if (offset + widths_size + heights_size + picanm_size > data.size()) {
        return std::unexpected(Error{"File too small for arrays"});
    }

    std::vector<uint16_t> widths(tile_count);
    for (size_t i = 0; i < tile_count; ++i) {
        widths[i] = read_u16(data, offset);
        offset += 2;
    }

    std::vector<uint16_t> heights(tile_count);
    for (size_t i = 0; i < tile_count; ++i) {
        heights[i] = read_u16(data, offset);
        offset += 2;
    }

    // Skip picanm
    offset += picanm_size;

    ArtFile art_file;
    art_file.tiles.reserve(tile_count);

    for (size_t i = 0; i < tile_count; ++i) {
        int w = widths[i];
        int h = heights[i];
        size_t pixel_count = static_cast<size_t>(w) * h;

        if (offset + pixel_count > data.size()) {
             return std::unexpected(Error{"File too small for pixel data"});
        }

        Tile tile;
        tile.width = w;
        tile.height = h;
        tile.indices.assign(data.begin() + offset, data.begin() + offset + pixel_count);
        
        art_file.tiles.push_back(std::move(tile));
        
        offset += pixel_count;
    }

    return art_file;
}

} // namespace art2img
