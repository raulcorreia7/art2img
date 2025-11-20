#pragma once

#include <vector>
#include "art2img/common.hpp"

namespace art2img {

struct Image {
    int width;
    int height;
    std::vector<Byte> rgba;
};

Result<std::vector<Byte>> encode_png(const Image& img);
Result<std::vector<Byte>> encode_bmp(const Image& img);
Result<std::vector<Byte>> encode_tga(const Image& img);

} // namespace art2img
