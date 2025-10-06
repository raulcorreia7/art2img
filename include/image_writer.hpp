#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "art_file.hpp"
#include "exceptions.hpp"
#include "palette.hpp"

namespace art2img {

enum class ImageFormat { PNG, TGA, BMP };

class ImageWriter {
public:
  struct Options {
    bool enable_alpha = true;        // Enable alpha channel support (PNG only)
    bool premultiply_alpha = false;  // Apply premultiplication for upscaling (PNG only)
    bool matte_hygiene = false;      // Apply alpha matte hygiene (erode + blur) (PNG only)
    bool fix_transparency = true;    // Enable magenta transparency processing (PNG only)

    Options() {}
  };

  // Write image to file
  static bool write_image(const std::filesystem::path& filename, ImageFormat format,
                          const Palette& palette, const ArtFile::Tile& tile,
                          const uint8_t* pixel_data, size_t pixel_data_size,
                          const Options& options = Options());

  // Write image to memory
  static bool write_image_to_memory(std::vector<uint8_t>& output, ImageFormat format,
                                    const Palette& palette, const ArtFile::Tile& tile,
                                    const uint8_t* pixel_data, size_t pixel_data_size,
                                    const Options& options = Options());

  // Public for testing
  static constexpr bool is_magenta(uint8_t r, uint8_t g, uint8_t b) {
    // Magenta detection: r8≥250, b8≥250, g8≤5
    return (r >= 250) && (b >= 250) && (g <= 5);
  }

private:
  // PNG operations
  static bool write_png_to_file(const std::filesystem::path& filename, const Palette& palette,
                                const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                size_t pixel_data_size, const Options& options);

  static bool write_png_to_memory(std::vector<uint8_t>& output, const Palette& palette,
                                  const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                  size_t pixel_data_size, const Options& options);

  // TGA operations
  static bool write_tga_to_file(const std::filesystem::path& filename, const Palette& palette,
                                const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                size_t pixel_data_size);

  static bool write_tga_to_memory(std::vector<uint8_t>& output, const Palette& palette,
                                  const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                  size_t pixel_data_size);

  // BMP operations
  static bool write_bmp_to_file(const std::filesystem::path& filename, const Palette& palette,
                                const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                size_t pixel_data_size);

  static bool write_bmp_to_memory(std::vector<uint8_t>& output, const Palette& palette,
                                  const ArtFile::Tile& tile, const uint8_t* pixel_data,
                                  size_t pixel_data_size);
};

}  // namespace art2img