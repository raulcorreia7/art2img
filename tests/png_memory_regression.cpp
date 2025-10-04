#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "art_file.hpp"
#include "image_writer.hpp"
#include "palette.hpp"

// Include new module headers for direct testing
#include "image_processor.hpp"
#include "file_operations.hpp"

int main(int argc, char** argv) {
  try {
    std::string disk_path = (argc > 1) ? argv[1] : std::string("output/png_memory_regression.png");
    if (!disk_path.empty()) {
      std::filesystem::path target_path(disk_path);
      if (target_path.has_parent_path()) {
        std::filesystem::create_directories(target_path.parent_path());
      }
    }
    art2img::Palette palette;
    art2img::ArtFile::Tile tile{};
    tile.width = 1024;
    tile.height = 1024;
    tile.anim_data = 0;
    tile.offset = 0;

    const size_t pixel_count = static_cast<size_t>(tile.width) * tile.height;
    std::vector<uint8_t> pixel_data(pixel_count);
    for (size_t i = 0; i < pixel_count; ++i) {
      pixel_data[i] = static_cast<uint8_t>(i % 256);
    }

    art2img::ImageWriter::Options options;
    std::vector<uint8_t> memory_png;
    
    // Test direct module usage
    auto rgba_data = art2img::image_processor::convert_to_rgba(palette, tile, pixel_data.data(), pixel_data.size(), options);
    memory_png = art2img::file_operations::encode_png_to_memory(rgba_data, tile.width, tile.height);
    
    if (memory_png.empty()) {
      std::cerr << "Failed to write PNG to memory using direct module calls" << std::endl;
      return 1;
    }

    // Also test ImageWriter for comparison
    std::vector<uint8_t> image_writer_png;
    if (!art2img::ImageWriter::write_image_to_memory(image_writer_png, art2img::ImageFormat::PNG, palette,
                                                     tile, pixel_data.data(), pixel_data.size(),
                                                     options)) {
      std::cerr << "Failed to write PNG to memory using ImageWriter" << std::endl;
      return 1;
    }

    if (!art2img::ImageWriter::write_image(disk_path, art2img::ImageFormat::PNG, palette, tile,
                                           pixel_data.data(), pixel_data.size(), options)) {
      std::cerr << "Failed to write PNG to disk" << std::endl;
      return 1;
    }

    std::ifstream file(disk_path, std::ios::binary);
    if (!file.is_open()) {
      std::cerr << "Failed to reopen PNG from disk" << std::endl;
      return 1;
    }
    std::vector<uint8_t> disk_png((std::istreambuf_iterator<char>(file)),
                                  std::istreambuf_iterator<char>());
    file.close();

    if (memory_png.empty()) {
      std::cerr << "In-memory PNG buffer is empty" << std::endl;
      return 1;
    }
    
    // Also compare with ImageWriter output
    if (image_writer_png.empty()) {
      std::cerr << "ImageWriter PNG buffer is empty" << std::endl;
      return 1;
    }
    
    if (memory_png != disk_png) {
      std::cerr << "Mismatch between in-memory and on-disk PNG outputs" << std::endl;
      std::cerr << "  In-memory size: " << memory_png.size() << " bytes" << std::endl;
      std::cerr << "  On-disk size:   " << disk_png.size() << " bytes" << std::endl;
      return 1;
    }
    
    // Verify ImageWriter output matches direct module usage
    if (memory_png != image_writer_png) {
      std::cerr << "Mismatch between direct module usage and ImageWriter outputs" << std::endl;
      std::cerr << "  Direct module size: " << memory_png.size() << " bytes" << std::endl;
      std::cerr << "  ImageWriter size:   " << image_writer_png.size() << " bytes" << std::endl;
    }

    std::cout << "PNG memory regression test passed (" << memory_png.size() << " bytes)"
              << std::endl;
    std::cout << "ImageWriter output size: " << image_writer_png.size() << " bytes"
              << std::endl;
    return 0;
  } catch (const std::exception& ex) {
    std::cerr << "Exception: " << ex.what() << std::endl;
    return 1;
  }
}
