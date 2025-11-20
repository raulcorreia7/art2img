#pragma once

#include <art2img/common.hpp>
#include <vector>
#include "art2img/palette.hpp"
#include "art2img/image.hpp"

namespace art2img {

struct Tile {
    int width;
    int height;
    std::vector<Byte> indices;
};

struct ArtFile {
    std::vector<Tile> tiles;
};

Result<ArtFile> parse_art(ByteSpan data);
Result<Image> render_tile(const Tile& tile, const Palette& palette);

} // namespace art2img
