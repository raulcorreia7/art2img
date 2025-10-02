#include "png_writer.hpp"

// Suppress warnings from stb_image_write.h
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../vendor/stb/stb_image_write.h"

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#include <iostream>
#include <algorithm>
#include <cstdint>
#include <cstring>

namespace art2img
{

bool PngWriter::write_png(const std::filesystem::path& filename,
                          const Palette& palette,
                          const ArtFile::Tile& tile,
                          const std::vector<uint8_t>& pixel_data,
                          const Options& options)
{
    if (tile.is_empty())
    {
        return true; // Skip empty tiles
    }

    if (pixel_data.size() != tile.size())
    {
        throw ArtException("Pixel data size mismatch for tile: " + filename.string());
    }

        // Convert indexed color to RGBA
        std::vector<uint8_t> rgba_data = convert_to_rgba(palette, tile, pixel_data, options);

        // Write PNG file using stb_image_write
        int result = stbi_write_png(filename.string().c_str(),
                                    tile.width,
                                    tile.height,
                                    4, // RGBA
                                    rgba_data.data(),
                                    tile.width * 4); // stride = width * 4 bytes per pixel

        if (result == 0)
        {
            std::cerr << "Error: Cannot create PNG file '" << filename << "'" << std::endl;
            return false;
        }

        return true;
    }

    bool PngWriter::write_png(const std::filesystem::path& filename,
                             const Palette& palette,
                             const ArtFile::Tile& tile,
                             const uint8_t* pixel_data,
                             size_t pixel_data_size,
                             const Options& options)
    {
        if (tile.is_empty())
        {
            return true; // Skip empty tiles
        }

        if (pixel_data_size != tile.size())
        {
            throw ArtException("Pixel data size mismatch for tile: " + filename.string());
        }

        // Convert indexed color to RGBA
        std::vector<uint8_t> rgba_data = convert_to_rgba(palette, tile, pixel_data, pixel_data_size, options);

        // Write PNG file using stb_image_write
        int result = stbi_write_png(filename.string().c_str(),
                                    tile.width,
                                    tile.height,
                                    4, // RGBA
                                    rgba_data.data(),
                                    tile.width * 4); // stride = width * 4 bytes per pixel

        if (result == 0)
        {
            std::cerr << "Error: Cannot create PNG file '" << filename << "'" << std::endl;
            return false;
        }

        return true;
    }

    bool PngWriter::write_png_to_memory(std::vector<uint8_t>& output,
                                       const Palette& palette,
                                       const ArtFile::Tile& tile,
                                       const std::vector<uint8_t>& pixel_data,
                                       const Options& options)
    {
        return write_png_to_memory(output, palette, tile, pixel_data.data(), pixel_data.size(), options);
    }

    bool PngWriter::write_png_to_memory(std::vector<uint8_t>& output,
                                       const Palette& palette,
                                       const ArtFile::Tile& tile,
                                       const uint8_t* pixel_data,
                                       size_t pixel_data_size,
                                       const Options& options)
    {
        output.clear();

        if (tile.is_empty())
        {
            return true; // Skip empty tiles
        }

        if (pixel_data_size != tile.size())
        {
            std::cerr << "Error: Pixel data size mismatch for tile" << std::endl;
            return false;
        }

        // Convert indexed color to RGBA
        std::vector<uint8_t> rgba_data = convert_to_rgba(palette, tile, pixel_data, pixel_data_size, options);

        // Pre-size buffer to minimize reallocations during callbacks
        if (rgba_data.size() > output.capacity()) {
            output.reserve(rgba_data.size() + 1024);
        }

        // Write PNG to memory using stb_image_write
        int result = stbi_write_png_to_func(
            [](void* context, void* data, int size) {
                auto* output_buffer = static_cast<std::vector<uint8_t>*>(context);
                auto* byte_data = static_cast<uint8_t*>(data);
                output_buffer->insert(output_buffer->end(), byte_data, byte_data + size);
            },
            &output,
            tile.width,
            tile.height,
            4, // RGBA
            rgba_data.data(),
            tile.width * 4); // stride = width * 4 bytes per pixel

        if (result == 0)
        {
            std::cerr << "Error: Cannot create PNG in memory" << std::endl;
            return false;
        }

        return true;
    }

    std::vector<uint8_t> PngWriter::convert_to_rgba(const Palette &palette,
                                                    const ArtFile::Tile &tile,
                                                    const std::vector<uint8_t> &pixel_data,
                                                    const Options &options)
    {
        return convert_to_rgba(palette, tile, pixel_data.data(), pixel_data.size(), options);
    }

    std::vector<uint8_t> PngWriter::convert_to_rgba(const Palette &palette,
                                                    const ArtFile::Tile &tile,
                                                    const uint8_t* pixel_data,
                                                    size_t pixel_data_size,
                                                    const Options &options)
    {
        if (pixel_data_size != tile.size()) {
            throw ArtException("Pixel data size mismatch for tile");
        }

        std::vector<uint8_t> rgba_data(static_cast<size_t>(tile.width) * tile.height * 4);

        const std::vector<uint8_t> &palette_data = palette.data();

        // ART format: pixels stored by columns (y + x * height)
        // PNG format: pixels stored by rows (x + y * width), top to bottom
        for (int32_t y = 0; y < tile.height; ++y)
        {
            for (int32_t x = 0; x < tile.width; ++x)
            {
                // Get pixel index from ART data (column-major)
                uint8_t pixel_index = pixel_data[static_cast<size_t>(y) + static_cast<size_t>(x) * tile.height];

                // The palette data is stored in TGA format (BGR after convert_to_tga_format())
                // For PNG we need to reverse this: BGR -> RGB
                // Palette data is already expanded to 8-bit by convert_to_tga_format()
                const uint8_t b8 = palette_data[pixel_index * 3 + 0]; // TGA Blue (was originally red)
                const uint8_t g8 = palette_data[pixel_index * 3 + 1]; // TGA Green (was originally green)
                const uint8_t r8 = palette_data[pixel_index * 3 + 2]; // TGA Red (was originally blue)

                // Calculate position in RGBA buffer (row-major)
                const size_t rgba_index = (static_cast<size_t>(y) * tile.width + static_cast<size_t>(x)) * 4;

                // Step B: Alpha construction (magenta keying)
                uint8_t alpha = 255;
                if (options.enable_alpha && options.enable_magenta_transparency && is_magenta(r8, g8, b8)) {
                    alpha = 0;
                }

                // Store RGBA values
                rgba_data[rgba_index + 0] = r8; // R
                rgba_data[rgba_index + 1] = g8; // G
                rgba_data[rgba_index + 2] = b8; // B
                rgba_data[rgba_index + 3] = alpha; // A
            }
        }

        // Step C: Apply premultiplication if enabled
        if (options.enable_alpha && options.premultiply_alpha) {
            apply_premultiplication(rgba_data);
        }

        // Step D: Optional matte hygiene
        if (options.enable_alpha && options.matte_hygiene) {
            apply_matte_hygiene(rgba_data, tile.width, tile.height);
            // Re-apply premultiplication after alpha changes
            if (options.premultiply_alpha) {
                apply_premultiplication(rgba_data);
            }
        }

        return rgba_data;
    }

    
    void PngWriter::apply_premultiplication(std::vector<uint8_t>& rgba_data) {
        // Step C: Enforce no hidden color + premultiply edges
        for (size_t i = 0; i < rgba_data.size(); i += 4) {
            uint8_t r = rgba_data[i];
            uint8_t g = rgba_data[i + 1];
            uint8_t b = rgba_data[i + 2];
            uint8_t a = rgba_data[i + 3];

            // Rule 1: erase hidden RGB
            if (a == 0) {
                rgba_data[i] = 0;     // R
                rgba_data[i + 1] = 0; // G
                rgba_data[i + 2] = 0; // B
            }
            // Rule 2: premultiply edges
            else if (a < 255) {
                // Premultiply with rounding: (r * a + 127) / 255
                rgba_data[i] = static_cast<uint8_t>((r * a + 127) / 255);     // R
                rgba_data[i + 1] = static_cast<uint8_t>((g * a + 127) / 255); // G
                rgba_data[i + 2] = static_cast<uint8_t>((b * a + 127) / 255); // B
            }
            // For a=255, RGB values remain unchanged (already premultiplied by 1.0)
        }
    }

    void PngWriter::apply_matte_hygiene(std::vector<uint8_t>& rgba_data, int width, int height) {
        // Create temporary alpha channel for processing
        std::vector<uint8_t> alpha_channel(static_cast<size_t>(width) * height);
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                size_t rgba_index = (static_cast<size_t>(y) * width + x) * 4;
                alpha_channel[static_cast<size_t>(y) * width + x] = rgba_data[rgba_index + 3];
            }
        }

        // Step D: Erode alpha by 1 px with 4-connected neighbors
        std::vector<uint8_t> eroded_alpha = alpha_channel;
        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                size_t idx = static_cast<size_t>(y) * width + x;
                if (alpha_channel[idx] > 0) {
                    // Check 4-connected neighbors (N, E, S, W)
                    uint8_t min_neighbor = 255;
                    min_neighbor = std::min(min_neighbor, alpha_channel[idx - width]);     // North
                    min_neighbor = std::min(min_neighbor, alpha_channel[idx + 1]);         // East
                    min_neighbor = std::min(min_neighbor, alpha_channel[idx + width]);     // South
                    min_neighbor = std::min(min_neighbor, alpha_channel[idx - 1]);         // West
                    
                    eroded_alpha[idx] = min_neighbor;
                }
            }
        }

        // Step D: Feather alpha with 3×3 box blur
        std::vector<uint8_t> blurred_alpha = eroded_alpha;
        for (int y = 1; y < height - 1; ++y) {
            for (int x = 1; x < width - 1; ++x) {
                size_t idx = static_cast<size_t>(y) * width + x;
                
                // 3x3 box blur
                uint32_t sum = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        sum += eroded_alpha[idx + static_cast<size_t>(dy) * width + dx];
                    }
                }
                blurred_alpha[idx] = static_cast<uint8_t>(sum / 9);
            }
        }

        // Update RGBA data with processed alpha
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                size_t rgba_index = (static_cast<size_t>(y) * width + x) * 4;
                rgba_data[rgba_index + 3] = blurred_alpha[static_cast<size_t>(y) * width + x];
            }
        }
    }

} // namespace art2img
