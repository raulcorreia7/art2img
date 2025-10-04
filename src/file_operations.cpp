#include "file_operations.hpp"

#include "image_processor.hpp"

// Suppress warnings from stb_image_write.h
#ifdef __GNUC__
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wmissing-field-initializers"
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

#ifdef __GNUC__
#  pragma GCC diagnostic pop
#endif

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

namespace art2img {
namespace file_operations {

// PNG Implementation
bool write_png_file(const std::filesystem::path& filename, const std::vector<uint8_t>& rgba_data,
                    int width, int height) {
  // Write PNG file using stb_image_write
  int result = stbi_write_png(filename.string().c_str(), width, height,
                              4,  // RGBA
                              rgba_data.data(),
                              width * 4);  // stride = width * 4 bytes per pixel

  if (result == 0) {
    std::cerr << "Error: Cannot create PNG file '" << filename << "'" << std::endl;
    return false;
  }

  return true;
}

std::vector<uint8_t> encode_png_to_memory(const std::vector<uint8_t>& rgba_data, int width,
                                          int height) {
  std::vector<uint8_t> output;

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
      &output, width, height,
      4,  // RGBA
      rgba_data.data(),
      width * 4);  // stride = width * 4 bytes per pixel

  if (result == 0) {
    std::cerr << "Error: Cannot create PNG in memory" << std::endl;
    return {};
  }

  return output;
}

// TGA Implementation
bool write_tga_file(const std::filesystem::path& filename, const Palette& palette,
                    const std::vector<uint8_t>& pixel_data, int width, int height) {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot create file '" << filename.string() << "'" << std::endl;
    return false;
  }

  // Create and write TGA header
  TgaHeader header = create_tga_header(width, height);
  std::vector<uint8_t> header_data = header.serialize();

  if (!file.write(reinterpret_cast<const char*>(header_data.data()), header_data.size())) {
    std::cerr << "Error: Cannot write TGA header to '" << filename << "'" << std::endl;
    return false;
  }

  // Write palette
  const std::vector<uint8_t> palette_data = palette.get_bgr_data();
  if (!file.write(reinterpret_cast<const char*>(palette_data.data()), palette_data.size())) {
    std::cerr << "Error: Cannot write palette to '" << filename << "'" << std::endl;
    return false;
  }

  // Write pixel data (ART stores by columns, TGA by rows from bottom)
  for (int32_t y = height - 1; y >= 0; --y) {
    for (int32_t x = 0; x < width; ++x) {
      // ART format: pixels stored by columns (y + x * height)
      uint8_t pixel = pixel_data[y + x * height];
      if (!file.write(reinterpret_cast<const char*>(&pixel), 1)) {
        std::cerr << "Error: Cannot write pixel data to '" << filename << "'" << std::endl;
        return false;
      }
    }
  }

  return true;
}

std::vector<uint8_t> encode_tga_to_memory(const Palette& palette,
                                          const std::vector<uint8_t>& pixel_data, int width,
                                          int height) {
  std::vector<uint8_t> output;

  // Create TGA header
  TgaHeader header = create_tga_header(width, height);
  std::vector<uint8_t> header_data = header.serialize();

  // Reserve space for the output buffer
  output.reserve(18 + 768 + pixel_data.size());  // header + palette + pixel data

  // Write TGA header
  output.insert(output.end(), header_data.begin(), header_data.end());

  // Write palette
  const std::vector<uint8_t> palette_data = palette.get_bgr_data();
  output.insert(output.end(), palette_data.begin(), palette_data.end());

  // Write pixel data (ART stores by columns, TGA by rows from bottom)
  for (int32_t y = height - 1; y >= 0; --y) {
    for (int32_t x = 0; x < width; ++x) {
      // ART format: pixels stored by columns (y + x * height)
      uint8_t pixel = pixel_data[y + x * height];
      output.push_back(pixel);
    }
  }

  return output;
}

TgaHeader create_tga_header(uint16_t width, uint16_t height) {
  TgaHeader header;
  header.id_length = 0;
  header.color_map_type = 1;      // Palette present
  header.image_type = 1;          // Color mapped image
  header.color_map_start = 0;     // First palette entry
  header.color_map_length = 256;  // 256 colors
  header.color_map_depth = 24;    // 24-bit palette entries
  header.x_origin = 0;
  header.y_origin = 0;
  header.width = width;
  header.height = height;
  header.pixel_depth = 8;          // 8-bit indexed color
  header.image_descriptor = 0x00;  // Bottom-left origin, no alpha

  return header;
}

std::vector<uint8_t> TgaHeader::serialize() const {
  std::vector<uint8_t> buffer(18);  // TGA header is 18 bytes

  buffer[0] = id_length;
  buffer[1] = color_map_type;
  buffer[2] = image_type;

  // Color map specification (5 bytes)
  write_little_endian_uint16(color_map_start, buffer, 3);
  write_little_endian_uint16(color_map_length, buffer, 5);
  buffer[7] = color_map_depth;

  // Image specification (10 bytes)
  write_little_endian_uint16(x_origin, buffer, 8);
  write_little_endian_uint16(y_origin, buffer, 10);
  write_little_endian_uint16(width, buffer, 12);
  write_little_endian_uint16(height, buffer, 14);
  buffer[16] = pixel_depth;
  buffer[17] = image_descriptor;

  return buffer;
}

void write_little_endian_uint16(uint16_t value, std::vector<uint8_t>& buffer, size_t offset) {
  buffer[offset] = static_cast<uint8_t>(value & 0xFF);
  buffer[offset + 1] = static_cast<uint8_t>((value >> 8) & 0xFF);
}

// BMP Implementation
bool write_bmp_file(const std::filesystem::path& filename, const Palette& palette,
                    const std::vector<uint8_t>& pixel_data, int width, int height,
                    const ImageWriter::Options& options) {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot create file '" << filename.string() << "'" << std::endl;
    return false;
  }

  // Convert indexed data to 32-bit BGRA pixels (BMP format is bottom-up)
  std::vector<uint32_t> bgra_pixels = convert_to_bgra(palette, pixel_data, width, height, options);

  // Calculate file size and headers
  uint32_t row_size = ((width * 32 + 31) / 32) * 4;  // 32-bit rows padded to 4-byte boundary
  uint32_t pixel_data_size_bytes = row_size * height;
  uint32_t file_size = 54 + pixel_data_size_bytes;  // 54 bytes for headers

  // Write BMP file header (14 bytes)
  uint8_t file_header[14] = {
      'B',
      'M',  // Signature
      static_cast<uint8_t>(file_size & 0xFF),
      static_cast<uint8_t>((file_size >> 8) & 0xFF),
      static_cast<uint8_t>((file_size >> 16) & 0xFF),
      static_cast<uint8_t>((file_size >> 24) & 0xFF),  // File size
      0,
      0,  // Reserved
      0,
      0,  // Reserved
      54,
      0,
      0,
      0  // Offset to pixel data (54 bytes)
  };
  file.write(reinterpret_cast<const char*>(file_header), 14);

  // Write BMP info header (40 bytes)
  uint8_t info_header[40] = {
      40,
      0,
      0,
      0,  // Info header size
      static_cast<uint8_t>(width & 0xFF),
      static_cast<uint8_t>((width >> 8) & 0xFF),
      static_cast<uint8_t>((width >> 16) & 0xFF),
      static_cast<uint8_t>((width >> 24) & 0xFF),  // Width
      static_cast<uint8_t>(height & 0xFF),
      static_cast<uint8_t>((height >> 8) & 0xFF),
      static_cast<uint8_t>((height >> 16) & 0xFF),
      static_cast<uint8_t>((height >> 24) & 0xFF),  // Height
      1,
      0,  // Planes
      32,
      0,  // Bits per pixel
      0,
      0,
      0,
      0,  // Compression (BI_RGB = 0)
      static_cast<uint8_t>(pixel_data_size_bytes & 0xFF),
      static_cast<uint8_t>((pixel_data_size_bytes >> 8) & 0xFF),
      static_cast<uint8_t>((pixel_data_size_bytes >> 16) & 0xFF),
      static_cast<uint8_t>((pixel_data_size_bytes >> 24) & 0xFF),  // Image size
      0x13,
      0x0B,
      0x00,
      0x00,  // X pixels per meter (~72 DPI)
      0x13,
      0x0B,
      0x00,
      0x00,  // Y pixels per meter (~72 DPI)
      0,
      0,
      0,
      0,  // Colors used
      0,
      0,
      0,
      0  // Important colors
  };
  file.write(reinterpret_cast<const char*>(info_header), 40);

  // Write pixel data (already in BGRA format)
  file.write(reinterpret_cast<const char*>(bgra_pixels.data()), pixel_data_size_bytes);

  return file.good();
}

std::vector<uint8_t> encode_bmp_to_memory(const Palette& palette,
                                          const std::vector<uint8_t>& pixel_data, int width,
                                          int height, const ImageWriter::Options& options) {
  std::vector<uint8_t> output;

  // Convert indexed data to 32-bit BGRA pixels (BMP format is bottom-up)
  std::vector<uint32_t> bgra_pixels = convert_to_bgra(palette, pixel_data, width, height, options);

  // Calculate file size and headers
  uint32_t row_size = ((width * 32 + 31) / 32) * 4;  // 32-bit rows padded to 4-byte boundary
  uint32_t pixel_data_size_bytes = row_size * height;
  uint32_t file_size = 54 + pixel_data_size_bytes;  // 54 bytes for headers

  // Reserve space for the output buffer
  output.reserve(file_size);

  // Create and write BMP file header (14 bytes)
  uint8_t file_header[14] = {
      'B',
      'M',  // Signature
      static_cast<uint8_t>(file_size & 0xFF),
      static_cast<uint8_t>((file_size >> 8) & 0xFF),
      static_cast<uint8_t>((file_size >> 16) & 0xFF),
      static_cast<uint8_t>((file_size >> 24) & 0xFF),  // File size
      0,
      0,  // Reserved
      0,
      0,  // Reserved
      54,
      0,
      0,
      0  // Offset to pixel data (54 bytes)
  };
  output.insert(output.end(), file_header, file_header + 14);

  // Create and write BMP info header (40 bytes)
  uint8_t info_header[40] = {
      40,
      0,
      0,
      0,  // Info header size
      static_cast<uint8_t>(width & 0xFF),
      static_cast<uint8_t>((width >> 8) & 0xFF),
      static_cast<uint8_t>((width >> 16) & 0xFF),
      static_cast<uint8_t>((width >> 24) & 0xFF),  // Width
      static_cast<uint8_t>(height & 0xFF),
      static_cast<uint8_t>((height >> 8) & 0xFF),
      static_cast<uint8_t>((height >> 16) & 0xFF),
      static_cast<uint8_t>((height >> 24) & 0xFF),  // Height
      1,
      0,  // Planes
      32,
      0,  // Bits per pixel
      0,
      0,
      0,
      0,  // Compression (BI_RGB = 0)
      static_cast<uint8_t>(pixel_data_size_bytes & 0xFF),
      static_cast<uint8_t>((pixel_data_size_bytes >> 8) & 0xFF),
      static_cast<uint8_t>((pixel_data_size_bytes >> 16) & 0xFF),
      static_cast<uint8_t>((pixel_data_size_bytes >> 24) & 0xFF),  // Image size
      0x13,
      0x0B,
      0x00,
      0x00,  // X pixels per meter (~72 DPI)
      0x13,
      0x0B,
      0x00,
      0x00,  // Y pixels per meter (~72 DPI)
      0,
      0,
      0,
      0,  // Colors used
      0,
      0,
      0,
      0  // Important colors
  };
  output.insert(output.end(), info_header, info_header + 40);

  // Write pixel data to output (already in BGRA format)
  output.insert(output.end(), reinterpret_cast<const uint8_t*>(bgra_pixels.data()),
                reinterpret_cast<const uint8_t*>(bgra_pixels.data()) + pixel_data_size_bytes);

  return output;
}

std::vector<uint32_t> convert_to_bgra(const Palette& palette,
                                      const std::vector<uint8_t>& pixel_data, int width, int height,
                                      const ImageWriter::Options& options) {
  // Convert indexed data to 32-bit BGRA pixels (BMP format is bottom-up)
  const std::vector<uint8_t> palette_bgr = palette.get_bgr_data();
  std::vector<uint32_t> bgra_pixels(width * height);

  // Process pixels from bottom to top (BMP format requirement)
  for (uint32_t y = 0; y < static_cast<uint32_t>(height); ++y) {
    uint32_t src_y = static_cast<uint32_t>(height) - 1 - y;  // Flip vertically
    const uint8_t* src_row = &pixel_data[src_y * width];
    uint32_t* dst_row = &bgra_pixels[y * width];

    for (uint32_t x = 0; x < static_cast<uint32_t>(width); ++x) {
      uint8_t pixel_index = src_row[x];

      // Get BGR values from palette (24-bit)
      uint8_t b8 = palette_bgr[pixel_index * 3 + 0];  // BGR Blue
      uint8_t g8 = palette_bgr[pixel_index * 3 + 1];  // BGR Green
      uint8_t r8 = palette_bgr[pixel_index * 3 + 2];  // BGR Red

      // Handle transparency keying for index 255
      uint8_t alpha = 255;
      if (options.fix_transparency && pixel_index == 255) {
        if (image_processor::is_build_engine_magenta(r8, g8, b8)) {
          alpha = 0;  // Transparent
        }
      }

      // Pack BGRA values into 32-bit integer (little-endian)
      uint32_t bgra_value = (static_cast<uint32_t>(alpha) << 24) |
                            (static_cast<uint32_t>(r8) << 16) | (static_cast<uint32_t>(g8) << 8) |
                            (static_cast<uint32_t>(b8) << 0);

      dst_row[x] = bgra_value;
    }
  }

  return bgra_pixels;
}

}  // namespace file_operations
}  // namespace art2img