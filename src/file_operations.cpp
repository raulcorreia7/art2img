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

BmpHeaders create_bmp_headers(int width, int height) {
  BmpHeaders headers;
  
  // Calculate row size (24-bit BMP: 3 bytes per pixel, padded to 4-byte boundary)
  headers.row_size = ((width * 3 + 3) / 4) * 4;  // 24-bit rows padded to 4-byte boundary
  headers.pixel_data_size = headers.row_size * height;
  uint32_t file_size = 54 + headers.pixel_data_size;  // 54 bytes for headers
  
  // Create BMP file header (14 bytes)
  headers.file_header = std::vector<uint8_t>(14);
  headers.file_header[0] = 'B';
  headers.file_header[1] = 'M';  // Signature
  headers.file_header[2] = static_cast<uint8_t>(file_size & 0xFF);
  headers.file_header[3] = static_cast<uint8_t>((file_size >> 8) & 0xFF);
  headers.file_header[4] = static_cast<uint8_t>((file_size >> 16) & 0xFF);
  headers.file_header[5] = static_cast<uint8_t>((file_size >> 24) & 0xFF);  // File size
  headers.file_header[6] = 0;
  headers.file_header[7] = 0;  // Reserved
  headers.file_header[8] = 0;
  headers.file_header[9] = 0;  // Reserved
  headers.file_header[10] = 54;
  headers.file_header[11] = 0;
  headers.file_header[12] = 0;
  headers.file_header[13] = 0;  // Offset to pixel data (54 bytes)
  
  // Create BMP info header (40 bytes)
  headers.info_header = std::vector<uint8_t>(40);
  headers.info_header[0] = 40;
  headers.info_header[1] = 0;
  headers.info_header[2] = 0;
  headers.info_header[3] = 0;  // Info header size
  headers.info_header[4] = static_cast<uint8_t>(width & 0xFF);
  headers.info_header[5] = static_cast<uint8_t>((width >> 8) & 0xFF);
  headers.info_header[6] = static_cast<uint8_t>((width >> 16) & 0xFF);
  headers.info_header[7] = static_cast<uint8_t>((width >> 24) & 0xFF);  // Width
  headers.info_header[8] = static_cast<uint8_t>(height & 0xFF);
  headers.info_header[9] = static_cast<uint8_t>((height >> 8) & 0xFF);
  headers.info_header[10] = static_cast<uint8_t>((height >> 16) & 0xFF);
  headers.info_header[11] = static_cast<uint8_t>((height >> 24) & 0xFF);  // Height
  headers.info_header[12] = 1;
  headers.info_header[13] = 0;  // Planes
  headers.info_header[14] = 24;
  headers.info_header[15] = 0;  // Bits per pixel (24-bit)
  headers.info_header[16] = 0;
  headers.info_header[17] = 0;
  headers.info_header[18] = 0;
  headers.info_header[19] = 0;  // Compression (BI_RGB = 0)
  headers.info_header[20] = static_cast<uint8_t>(headers.pixel_data_size & 0xFF);
  headers.info_header[21] = static_cast<uint8_t>((headers.pixel_data_size >> 8) & 0xFF);
  headers.info_header[22] = static_cast<uint8_t>((headers.pixel_data_size >> 16) & 0xFF);
  headers.info_header[23] = static_cast<uint8_t>((headers.pixel_data_size >> 24) & 0xFF);  // Image size
  headers.info_header[24] = 0x13;
  headers.info_header[25] = 0x0B;
  headers.info_header[26] = 0x00;
  headers.info_header[27] = 0x00;  // X pixels per meter (~72 DPI)
  headers.info_header[28] = 0x13;
  headers.info_header[29] = 0x0B;
  headers.info_header[30] = 0x00;
  headers.info_header[31] = 0x00;  // Y pixels per meter (~72 DPI)
  headers.info_header[32] = 0;
  headers.info_header[33] = 0;
  headers.info_header[34] = 0;
  headers.info_header[35] = 0;  // Colors used (0 = default)
  headers.info_header[36] = 0;
  headers.info_header[37] = 0;
  headers.info_header[38] = 0;
  headers.info_header[39] = 0;  // Important colors (0 = all)
  
  return headers;
}

void write_bmp_pixels_direct(std::ostream& output, const Palette& palette,
                             const std::vector<uint8_t>& pixel_data, int width, int height,
                             const BmpHeaders& headers) {
  // Get palette BGR data
  const std::vector<uint8_t> palette_bgr = palette.get_bgr_data();
  
  // Write pixel data row by row (BMP is bottom-up)
  for (int y = 0; y < height; ++y) {
    // BMP stores rows bottom-to-top, so we process from bottom row
    // For vertical flip, we need to process from the opposite end
    int flipped_y = height - 1 - y;
    
    // Write pixels for this row
    for (int x = 0; x < width; ++x) {
      // Get pixel index from ART format (column-major storage)
      // Apply vertical flip by using flipped_y instead of y
      uint8_t pixel_index = pixel_data[flipped_y + x * height];
      
      // Get BGR values from palette
      uint8_t b = palette_bgr[pixel_index * 3 + 0];  // BGR Blue
      uint8_t g = palette_bgr[pixel_index * 3 + 1];  // BGR Green
      uint8_t r = palette_bgr[pixel_index * 3 + 2];  // BGR Red
      
      // Preserve Build Engine transparency color (252, 0, 252)
      if (image_processor::is_build_engine_magenta(r, g, b)) {
        r = 252;
        g = 0;
        b = 252;
      }
      
      // Write BGR pixel data directly to output
      output.put(static_cast<char>(b));  // Blue
      output.put(static_cast<char>(g));  // Green
      output.put(static_cast<char>(r));  // Red
    }
    
    // Add padding bytes if needed
    uint32_t padding = headers.row_size - (width * 3);
    for (uint32_t p = 0; p < padding; ++p) {
      output.put(0);  // Pad with zeros
    }
  }
}

bool write_bmp_file(const std::filesystem::path& filename, const Palette& palette,
                    const std::vector<uint8_t>& pixel_data, int width, int height) {
  std::ofstream file(filename, std::ios::binary);
  if (!file.is_open()) {
    std::cerr << "Error: Cannot create file '" << filename.string() << "'" << std::endl;
    return false;
  }
  
  // Create unified headers
  BmpHeaders headers = create_bmp_headers(width, height);
  
  // Write headers
  file.write(reinterpret_cast<const char*>(headers.file_header.data()), headers.file_header.size());
  file.write(reinterpret_cast<const char*>(headers.info_header.data()), headers.info_header.size());
  
  // Write pixel data directly
  write_bmp_pixels_direct(file, palette, pixel_data, width, height, headers);
  
  return file.good();
}

std::vector<uint8_t> encode_bmp_to_memory(const Palette& palette,
                                          const std::vector<uint8_t>& pixel_data, int width,
                                          int height) {
  std::vector<uint8_t> output;
  
  // Create unified headers
  BmpHeaders headers = create_bmp_headers(width, height);
  
  // Reserve space for the output buffer
  output.reserve(54 + headers.pixel_data_size);
  
  // Write headers
  output.insert(output.end(), headers.file_header.begin(), headers.file_header.end());
  output.insert(output.end(), headers.info_header.begin(), headers.info_header.end());
  
  // Write pixel data to a stringstream and then copy to output
  std::ostringstream pixel_stream(std::ios::binary);
  write_bmp_pixels_direct(pixel_stream, palette, pixel_data, width, height, headers);
  
  // Get the pixel data and append to output
  std::string pixel_data_str = pixel_stream.str();
  output.insert(output.end(), pixel_data_str.begin(), pixel_data_str.end());
  
  return output;
}

}  // namespace file_operations
}  // namespace art2img