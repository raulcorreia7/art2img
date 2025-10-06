#pragma once

#include <cstdint>
#include <vector>

#include "art_file.hpp"
#include "image_writer.hpp"
#include "palette.hpp"

namespace art2img {
namespace image_processor {

/**
 * @brief Convert indexed color data to RGBA format
 *
 * @param palette The color palette to use for conversion
 * @param tile The tile information containing dimensions
 * @param pixel_data The input indexed pixel data
 * @param pixel_data_size Size of the pixel data
 * @param options Image processing options
 * @return std::vector<uint8_t> RGBA data (4 bytes per pixel)
 */
std::vector<uint8_t> convert_to_rgba(const Palette& palette, const ArtFile::Tile& tile,
                                     const uint8_t* pixel_data, size_t pixel_data_size,
                                     const ImageWriter::Options& options);

/**
 * @brief Apply alpha premultiplication to RGBA data
 *
 * @param rgba_data RGBA data to modify in-place
 */
void apply_premultiplication(std::vector<uint8_t>& rgba_data);

/**
 * @brief Apply matte hygiene (erode + blur) to alpha channel
 *
 * @param rgba_data RGBA data to modify in-place
 * @param width Image width
 * @param height Image height
 */
void apply_matte_hygiene(std::vector<uint8_t>& rgba_data, int width, int height);

/**
 * @brief Check if a color is the Build Engine transparency color (252, 0, 252)
 *
 * The Build Engine uses a specific magenta color (252, 0, 252) for transparency,
 * which is slightly different from the general magenta detection.
 * This function is used for PNG and TGA formats with alpha channel support.
 * For BMP files, this exact color should be preserved rather than converted to black.
 *
 * @param r Red component
 * @param g Green component
 * @param b Blue component
 * @return true if the color is the Build Engine transparency magenta
 * @return false otherwise
 */
constexpr bool is_build_engine_magenta(uint8_t r, uint8_t g, uint8_t b) {
  // Magenta detection: r8≥250, b8≥250, g8≤5
  return (r >= 250) && (b >= 250) && (g <= 5);
}

}  // namespace image_processor
}  // namespace art2img