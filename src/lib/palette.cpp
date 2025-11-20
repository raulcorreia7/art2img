#include "art2img/palette.hpp"
#include <fstream>
#include <vector>

namespace art2img {

Result<Palette> load_palette(const std::filesystem::path& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        return std::unexpected(Error{"Failed to open file: " + path.string()});
    }

    // Check file size
    file.seekg(0, std::ios::end);
    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    if (size < 768) {
        return std::unexpected(Error{"Palette file too small"});
    }

    std::vector<uint8_t> buffer(768);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), 768)) {
        return std::unexpected(Error{"Failed to read palette data"});
    }

    Palette palette;
    palette.colors.reserve(256);

    for (int i = 0; i < 256; ++i) {
        uint8_t r = buffer[i * 3 + 0];
        uint8_t g = buffer[i * 3 + 1];
        uint8_t b = buffer[i * 3 + 2];

        // Scale 0-63 to 0-255
        r = (r << 2) | (r >> 4);
        g = (g << 2) | (g >> 4);
        b = (b << 2) | (b >> 4);

        uint8_t a = (i == 255) ? 0 : 255;

        palette.colors.push_back({r, g, b, a});
    }

    return palette;
}

} // namespace art2img
