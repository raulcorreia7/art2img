#pragma once

#include <vector>
#include <filesystem>
#include "art2img/common.hpp"

namespace art2img {

struct Image {
    int width;
    int height;
    std::vector<Byte> rgba;
};

Result<void> save_png(const Image& img, const std::filesystem::path& path);
Result<void> save_bmp(const Image& img, const std::filesystem::path& path);
Result<void> save_tga(const Image& img, const std::filesystem::path& path);

} // namespace art2img
