#include "art2img/image.hpp"

#undef STB_IMAGE_WRITE_IMPLEMENTATION
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#include <stb_image_write.h>
#pragma GCC diagnostic pop

namespace art2img {

namespace {
void write_func(void *context, void *data, int size) {
    auto* buffer = static_cast<std::vector<Byte>*>(context);
    auto* bytes = static_cast<const Byte*>(data);
    buffer->insert(buffer->end(), bytes, bytes + size);
}
}

Result<std::vector<Byte>> encode_png(const Image& img) {
    std::vector<Byte> buffer;
    if (stbi_write_png_to_func(write_func, &buffer, img.width, img.height, 4, img.rgba.data(), img.width * 4)) {
        return buffer;
    }
    return std::unexpected(Error(core::errc::io_failure, "Failed to encode PNG"));
}

Result<std::vector<Byte>> encode_bmp(const Image& img) {
    std::vector<Byte> buffer;
    if (stbi_write_bmp_to_func(write_func, &buffer, img.width, img.height, 4, img.rgba.data())) {
        return buffer;
    }
    return std::unexpected(Error(core::errc::io_failure, "Failed to encode BMP"));
}

Result<std::vector<Byte>> encode_tga(const Image& img) {
    std::vector<Byte> buffer;
    if (stbi_write_tga_to_func(write_func, &buffer, img.width, img.height, 4, img.rgba.data())) {
        return buffer;
    }
    return std::unexpected(Error(core::errc::io_failure, "Failed to encode TGA"));
}

} // namespace art2img
