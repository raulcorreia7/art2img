#pragma once

#include <filesystem>
#include <string>
#include <vector>
#include <cstdint>

#include "art_file.hpp"
#include "palette.hpp"
#include "image_writer.hpp"

namespace art2img {
namespace file_operations {

// PNG operations
bool write_png_file(const std::filesystem::path& filename, 
                   const std::vector<uint8_t>& rgba_data,
                   int width, int height);

std::vector<uint8_t> encode_png_to_memory(const std::vector<uint8_t>& rgba_data,
                                         int width, int height);

// TGA operations
struct TgaHeader {
  uint8_t id_length;          // Image ID field length (0)
  uint8_t color_map_type;     // Color map type (1 = palette)
  uint8_t image_type;         // Image type (1 = color mapped)
  uint16_t color_map_start;   // First color map entry (0)
  uint16_t color_map_length;  // Color map length (256)
  uint8_t color_map_depth;    // Color map entry size (24 bits)
  uint16_t x_origin;          // X origin (0)
  uint16_t y_origin;          // Y origin (0)
  uint16_t width;             // Image width
  uint16_t height;            // Image height
  uint8_t pixel_depth;        // Bits per pixel (8)
  uint8_t image_descriptor;   // Image descriptor (0x00)

  std::vector<uint8_t> serialize() const;
};

bool write_tga_file(const std::filesystem::path& filename,
                   const Palette& palette,
                   const std::vector<uint8_t>& pixel_data,
                   int width, int height);

std::vector<uint8_t> encode_tga_to_memory(const Palette& palette,
                                         const std::vector<uint8_t>& pixel_data,
                                         int width, int height);

// BMP operations
struct BmpFileHeader {
  uint16_t bfType{0x4D42};      // "BM"
  uint32_t bfSize{0};           // Total file size
  uint16_t bfReserved1{0};
  uint16_t bfReserved2{0};
  uint32_t bfOffBits{54};       // Offset to pixel data (54 for 32-bit)
};

struct BmpInfoHeader {
  uint32_t biSize{40};          // Size of this header (40 bytes)
  int32_t  biWidth{0};
  int32_t  biHeight{0};
  uint16_t biPlanes{1};
  uint16_t biBitCount{32};      // 32 bits per pixel
  uint32_t biCompression{0};     // BI_RGB = 0 (no compression)
  uint32_t biSizeImage{0};      // Can be 0 for uncompressed
  int32_t  biXPelsPerMeter{2835}; // ~72 DPI
  int32_t  biYPelsPerMeter{2835}; // ~72 DPI
  uint32_t biClrUsed{0};
  uint32_t biClrImportant{0};
};

bool write_bmp_file(const std::filesystem::path& filename,
                   const Palette& palette,
                   const std::vector<uint8_t>& pixel_data,
                   int width, int height,
                   const ImageWriter::Options& options);

std::vector<uint8_t> encode_bmp_to_memory(const Palette& palette,
                                         const std::vector<uint8_t>& pixel_data,
                                         int width, int height,
                                         const ImageWriter::Options& options);

// Helper functions
TgaHeader create_tga_header(uint16_t width, uint16_t height);
void write_little_endian_uint16(uint16_t value, std::vector<uint8_t>& buffer,
                               size_t offset);
std::vector<uint32_t> convert_to_bgra(const Palette& palette,
                                    const std::vector<uint8_t>& pixel_data,
                                    int width, int height,
                                    const ImageWriter::Options& options);

}  // namespace file_operations
}  // namespace art2img