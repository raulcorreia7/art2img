#pragma once

#include <vector>
#include <cstdint>
#include "art2img/common.hpp"

namespace art2img {

struct Color {
    uint8_t r, g, b, a;
};

struct Palette {
    std::vector<Color> colors;
};

Result<Palette> parse_palette(ByteSpan data);

} // namespace art2img
