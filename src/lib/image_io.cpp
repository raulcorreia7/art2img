#include "art2img/image.hpp"

#undef STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <stb_image_write.h>
#pragma GCC diagnostic pop

namespace art2img {

Result<void> save_png(const Image& img, const std::filesystem::path& path) {
    if (stbi_write_png(path.string().c_str(), img.width, img.height, 4, img.rgba.data(), img.width * 4)) {
        return {};
    }
    return std::unexpected(core::make_error(core::errc::io_failure, "Failed to save PNG"));
}

Result<void> save_bmp(const Image& img, const std::filesystem::path& path) {
    if (stbi_write_bmp(path.string().c_str(), img.width, img.height, 4, img.rgba.data())) {
        return {};
    }
    return std::unexpected(core::make_error(core::errc::io_failure, "Failed to save BMP"));
}

Result<void> save_tga(const Image& img, const std::filesystem::path& path) {
    if (stbi_write_tga(path.string().c_str(), img.width, img.height, 4, img.rgba.data())) {
        return {};
    }
    return std::unexpected(core::make_error(core::errc::io_failure, "Failed to save TGA"));
}

} // namespace art2img
