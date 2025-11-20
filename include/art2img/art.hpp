#pragma once

#include <art2img/common.hpp>
#include <vector>

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

} // namespace art2img
