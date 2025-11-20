#pragma once

#include <vector>
#include <cstdint>
#include <filesystem>
#include "art2img/common.hpp"

namespace art2img {

struct Color {
    uint8_t r, g, b, a;
};

struct Palette {
    std::vector<Color> colors;
};

Result<Palette> load_palette(const std::filesystem::path& path);

} // namespace art2img
