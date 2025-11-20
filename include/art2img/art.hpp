#pragma once

#include <art2img/common.hpp>
#include <filesystem>
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

Result<ArtFile> load_art(const std::filesystem::path& path);

} // namespace art2img
