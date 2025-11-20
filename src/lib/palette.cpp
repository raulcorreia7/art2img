#include "art2img/palette.hpp"
#include <vector>

namespace art2img {

Result<Palette> parse_palette(ByteSpan data) {
    if (data.size() < 768) {
        return std::unexpected(Error{"Palette data too small"});
    }

    Palette palette;
    palette.colors.reserve(256);

    for (int i = 0; i < 256; ++i) {
        uint8_t r = std::to_integer<uint8_t>(data[i * 3 + 0]);
        uint8_t g = std::to_integer<uint8_t>(data[i * 3 + 1]);
        uint8_t b = std::to_integer<uint8_t>(data[i * 3 + 2]);

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
