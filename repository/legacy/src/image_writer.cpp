#include "art2img/image_writer.hpp"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>

#include "art2img/file_operations.hpp"
#include "art2img/image_processor.hpp"

namespace art2img {

bool ImageWriter::write_image(const std::filesystem::path& filename, ImageFormat format,
                              const Palette& palette, const ArtFile::Tile& tile,
                              const uint8_t* pixel_data, size_t pixel_data_size,
                              const Options& options) {
  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    throw ArtException("Pixel data size mismatch for tile: " + filename.string());
  }

  switch (format) {
  case ImageFormat::PNG:
    return write_png_to_file(filename, palette, tile, pixel_data, pixel_data_size, options);
  case ImageFormat::TGA:
    return write_tga_to_file(filename, palette, tile, pixel_data, pixel_data_size);
  case ImageFormat::BMP:
    return write_bmp_to_file(filename, palette, tile, pixel_data, pixel_data_size);
  default:
    throw ArtException("Unsupported image format: " + std::to_string(static_cast<int>(format)));
  }
}

bool ImageWriter::write_image_to_memory(std::vector<uint8_t>& output, ImageFormat format,
                                        const Palette& palette, const ArtFile::Tile& tile,
                                        const uint8_t* pixel_data, size_t pixel_data_size,
                                        const Options& options) {
  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    std::cerr << "Error: Pixel data size mismatch for tile" << std::endl;
    return false;
  }

  switch (format) {
  case ImageFormat::PNG:
    return write_png_to_memory(output, palette, tile, pixel_data, pixel_data_size, options);
  case ImageFormat::TGA:
    return write_tga_to_memory(output, palette, tile, pixel_data, pixel_data_size);
  case ImageFormat::BMP:
    return write_bmp_to_memory(output, palette, tile, pixel_data, pixel_data_size);
  default:
    std::cerr << "Error: Unsupported image format" << std::endl;
    return false;
  }
}

// PNG Implementation

bool ImageWriter::write_png_to_file(const std::filesystem::path& filename, const Palette& palette,
                                    const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                    size_t pixel_data_size, const Options& options) {
  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    throw ArtException("Pixel data size mismatch for tile: " + filename.string());
  }

  // For PNG, we want clean transparency without matte hygiene or premultiplication that might cause
  // shimmering
  Options png_options = options;
  png_options.matte_hygiene = false;
  png_options.premultiply_alpha = false;

  // Convert indexed color to RGBA
  std::vector<uint8_t> rgba_data =
      image_processor::convert_to_rgba(palette, tile, pixel_data, pixel_data_size, png_options);

  // Write PNG file using file_operations
  return file_operations::write_png_file(filename, rgba_data, tile.width, tile.height);
}

bool ImageWriter::write_png_to_memory(std::vector<uint8_t>& output, const Palette& palette,
                                      const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                      size_t pixel_data_size, const Options& options) {
  output.clear();

  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    std::cerr << "Error: Pixel data size mismatch for tile" << std::endl;
    return false;
  }

  // Convert indexed color to RGBA
  std::vector<uint8_t> rgba_data =
      image_processor::convert_to_rgba(palette, tile, pixel_data, pixel_data_size, options);

  // Encode PNG to memory using file_operations
  output = file_operations::encode_png_to_memory(rgba_data, tile.width, tile.height);
  return !output.empty();
}

// TGA Implementation

bool ImageWriter::write_tga_to_file(const std::filesystem::path& filename, const Palette& palette,
                                    const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                    size_t pixel_data_size) {
  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    throw ArtException("Pixel data size mismatch for tile: " + filename.string());
  }

  // Write TGA file using file_operations
  return file_operations::write_tga_file(
      filename, palette, std::vector<uint8_t>(pixel_data, pixel_data + pixel_data_size), tile.width,
      tile.height);
}

bool ImageWriter::write_tga_to_memory(std::vector<uint8_t>& output, const Palette& palette,
                                      const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                      size_t pixel_data_size) {
  output.clear();

  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    std::cerr << "Error: Pixel data size mismatch for tile" << std::endl;
    return false;
  }

  // Encode TGA to memory using file_operations
  output = file_operations::encode_tga_to_memory(
      palette, std::vector<uint8_t>(pixel_data, pixel_data + pixel_data_size), tile.width,
      tile.height);
  return !output.empty();
}

// BMP Implementation

bool ImageWriter::write_bmp_to_file(const std::filesystem::path& filename, const Palette& palette,
                                    const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                    size_t pixel_data_size) {
  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    throw ArtException("Pixel data size mismatch for tile: " + filename.string());
  }

  // Write BMP file using file_operations
  return file_operations::write_bmp_file(
      filename, palette, std::vector<uint8_t>(pixel_data, pixel_data + pixel_data_size), tile.width,
      tile.height);
}

bool ImageWriter::write_bmp_to_memory(std::vector<uint8_t>& output, const Palette& palette,
                                      const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                      size_t pixel_data_size) {
  output.clear();

  if (tile.is_empty()) {
    return true;  // Skip empty tiles
  }

  if (pixel_data_size != tile.size()) {
    std::cerr << "Error: Pixel data size mismatch for tile" << std::endl;
    return false;
  }

  // Encode BMP to memory using file_operations
  output = file_operations::encode_bmp_to_memory(
      palette, std::vector<uint8_t>(pixel_data, pixel_data + pixel_data_size), tile.width,
      tile.height);
  return !output.empty();
}

}  // namespace art2img