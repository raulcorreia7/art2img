#include "extractor_api.hpp"

#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>

#include "image_writer.hpp"

namespace art2img {

ExtractorAPI::ExtractorAPI() {
  // Initialize with default palette
  palette_ = std::make_unique<Palette>();
}

bool ExtractorAPI::load_art_file(const std::filesystem::path& filename) {
  try {
    art_file_ = std::make_unique<ArtFile>();
    if (!art_file_->load(filename)) {
      art_file_.reset();
      return false;
    }
    return true;
  } catch (const ArtException& e) {
    art_file_.reset();
    throw;
  }
}

bool ExtractorAPI::load_palette_file(const std::filesystem::path& filename) {
  if (!palette_) {
    palette_ = std::make_unique<Palette>();
  }
  try {
    return palette_->load_from_file(filename);
  } catch (const ArtException& e) {
    throw;
  }
}

bool ExtractorAPI::load_art_from_memory(const uint8_t* data, size_t size) {
  try {
    art_file_ = std::make_unique<ArtFile>(data, size);
    return true;
  } catch (const ArtException&) {
    art_file_.reset();
    return false;
  }
}

bool ExtractorAPI::load_palette_from_memory(const uint8_t* data, size_t size) {
  if (!palette_) {
    palette_ = std::make_unique<Palette>();
  }
  return palette_->load_from_memory(data, size);
}

void ExtractorAPI::set_duke3d_default_palette() {
  if (!palette_) {
    palette_ = std::make_unique<Palette>();
  }
  palette_->load_duke3d_default();
}

void ExtractorAPI::set_blood_default_palette() {
  if (!palette_) {
    palette_ = std::make_unique<Palette>();
  }
  palette_->load_blood_default();
}

ExtractionResult ExtractorAPI::extract_tile(uint32_t tile_index, ImageFormat format,
                                            ImageWriter::Options options) {
  ExtractionResult result;
  result.success = false;
  result.tile_index = tile_index;
  switch (format) {
  case ImageFormat::PNG:
    result.format = "png";
    break;
  case ImageFormat::TGA:
    result.format = "tga";
    break;
  case ImageFormat::BMP:
    result.format = "bmp";
    break;
  default:
    result.format = "unknown";
    break;
  }

  if (!art_file_ || !palette_) {
    result.error_message = "ART file or palette not loaded";
    return result;
  }

  const auto& tiles = art_file_->tiles();
  if (tile_index >= tiles.size()) {
    result.error_message = "Tile index out of range";
    return result;
  }

  const auto& tile = tiles[tile_index];
  result.width = tile.width;
  result.height = tile.height;

  // Set animation data
  result.anim_frames = tile.anim_frames();
  result.anim_type = tile.anim_type();
  result.x_offset = tile.x_offset();
  result.y_offset = tile.y_offset();
  result.anim_speed = tile.anim_speed();
  result.other_flags = tile.other_flags();

  if (tile.is_empty()) {
    result.success = true;
    return result;
  }

  // Get direct pointer to tile data (zero-copy)
  const uint8_t* pixel_data = nullptr;
  size_t pixel_data_size = 0;

  if (!tile.is_empty()) {
    if (!art_file_->has_data()) {
      result.error_message = "ART data not loaded in memory";
      return result;
    }

    if (tile.offset + tile.size() > art_file_->data_size()) {
      result.error_message = "Tile data extends beyond buffer size";
      return result;
    }

    pixel_data = art_file_->data() + tile.offset;
    pixel_data_size = tile.size();
  }

  // Extract to image in memory
  try {
    if (tile.is_empty()) {
      // Empty tile - no image data
      result.success = true;
      return result;
    }

    if (!ImageWriter::write_image_to_memory(result.image_data, format, *palette_, tile, pixel_data,
                                            pixel_data_size, options)) {
      result.error_message = "Failed to write image to memory";
      return result;
    }
  } catch (const ArtException& e) {
    result.error_message = "Failed to write image to memory: " + std::string(e.what());
    return result;
  }

  result.success = true;
  return result;
}

ExtractionResult ExtractorAPI::extract_tile_png(uint32_t tile_index, ImageWriter::Options options) {
  return extract_tile(tile_index, ImageFormat::PNG, options);
}

ExtractionResult ExtractorAPI::extract_tile_tga(uint32_t tile_index, ImageWriter::Options options) {
  return extract_tile(tile_index, ImageFormat::TGA, options);
}

ExtractionResult ExtractorAPI::extract_tile_bmp(uint32_t tile_index, ImageWriter::Options options) {
  return extract_tile(tile_index, ImageFormat::BMP, options);
}

std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles(ImageFormat format,
                                                              ImageWriter::Options options) {
  std::vector<ExtractionResult> results;

  if (!art_file_ || !palette_) {
    ExtractionResult result;
    result.success = false;
    result.error_message = "ART file or palette not loaded";
    results.push_back(result);
    return results;
  }

  uint32_t tile_count = static_cast<uint32_t>(art_file_->tiles().size());
  results.reserve(tile_count);

  for (uint32_t i = 0; i < tile_count; ++i) {
    results.push_back(extract_tile(i, format, options));
  }

  return results;
}

std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_png(ImageWriter::Options options) {
  return extract_all_tiles(ImageFormat::PNG, options);
}

std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_tga(ImageWriter::Options options) {
  return extract_all_tiles(ImageFormat::TGA, options);
}

std::vector<ExtractionResult> ExtractorAPI::extract_all_tiles_bmp(ImageWriter::Options options) {
  return extract_all_tiles(ImageFormat::BMP, options);
}

ArtView ExtractorAPI::get_art_view() const {
  ArtView view;

  if (!art_file_ || !palette_) {
    throw ArtException("ART file or palette not loaded");
  }

  // Set art data pointer and size
  if (art_file_->is_open() && !art_file_->has_data()) {
    // File-based mode, we need to read the entire file into memory
    throw ArtException("get_art_view() requires memory-based loading, not file-based");
  }

  view.art_data = art_file_->data();
  view.art_size = art_file_->data_size();
  view.palette = palette_.get();
  view.header = art_file_->header();
  view.tiles = art_file_->tiles();  // Copy the tiles vector (metadata only)

  return view;
}

// ImageView method implementations

bool ImageView::save_to_image(const std::filesystem::path& path, ImageFormat format,
                              ImageWriter::Options options) const {
  if (!parent || !parent->palette) {
    throw ArtException("Invalid ImageView state: parent or palette is null");
  }

  const uint8_t* pixels = pixel_data();
  if (!pixels) {
    // Empty tile - skip
    return true;
  }

  ArtFile::Tile tile = parent->get_tile(tile_index);
  return ImageWriter::write_image(path, format, *parent->palette, tile, pixels, size(), options);
}

bool ImageView::save_to_png(const std::filesystem::path& path, ImageWriter::Options options) const {
  return save_to_image(path, ImageFormat::PNG, options);
}

std::vector<uint8_t> ImageView::extract_to_image(ImageFormat format,
                                                 ImageWriter::Options options) const {
  if (!parent || !parent->palette) {
    throw ArtException("Invalid ImageView state: parent or palette is null");
  }

  const uint8_t* pixels = pixel_data();
  if (!pixels) {
    // Empty tile - return empty vector
    return {};
  }

  ArtFile::Tile tile = parent->get_tile(tile_index);
  std::vector<uint8_t> result;
  if (!ImageWriter::write_image_to_memory(result, format, *parent->palette, tile, pixels, size(),
                                          options)) {
    throw ArtException("Failed to extract image to memory");
  }
  return result;
}

std::vector<uint8_t> ImageView::extract_to_png(ImageWriter::Options options) const {
  return extract_to_image(ImageFormat::PNG, options);
}

bool ImageView::save_to_tga(const std::filesystem::path& path) const {
  return save_to_image(path, ImageFormat::TGA);
}

bool ImageView::save_to_bmp(const std::filesystem::path& path, ImageWriter::Options options) const {
  return save_to_image(path, ImageFormat::BMP, options);
}

std::vector<uint8_t> ImageView::extract_to_tga() const {
  return extract_to_image(ImageFormat::TGA);
}

bool ExtractorAPI::write_animation_data(const std::string& art_file_path,
                                        const std::string& output_dir) const {
  if (!is_art_loaded() || !art_file_) {
    return false;
  }

  std::string filename = output_dir + "/animdata.ini";
  std::ofstream file(filename, std::ios::app);  // Append mode

  if (!file.is_open()) {
    return false;
  }

  // Write header for this ART file
  file << "; Animation data from \"" << art_file_path << "\"\n"
       << "; Extracted by art2img\n"
       << "\n";

  // Get the art file header and tiles
  const auto& tiles = art_file_->tiles();
  const auto& header = art_file_->header();

  for (uint32_t i = 0; i < tiles.size(); ++i) {
    const auto& tile = tiles[i];

    if (tile.anim_data != 0) {
      // Check if tile has meaningful animation data
      if (tile.anim_frames() != 0 || tile.anim_type() != 0 || tile.anim_speed() != 0) {
        file << "[tile" << std::setw(4) << std::setfill('0') << (i + header.start_tile)
             << ".tga -> tile" << std::setw(4) << std::setfill('0')
             << (i + header.start_tile + tile.anim_frames()) << ".tga]\n";
        file << "   AnimationType=" << get_animation_type_string(tile.anim_type()) << "\n";
        file << "   AnimationSpeed=" << tile.anim_speed() << "\n";
        file << "\n";
      }

      file << "[tile" << std::setw(4) << std::setfill('0') << (i + header.start_tile) << ".tga]\n";
      file << "   XCenterOffset=" << static_cast<int>(tile.x_offset()) << "\n";
      file << "   YCenterOffset=" << static_cast<int>(tile.y_offset()) << "\n";
      file << "   OtherFlags=" << tile.other_flags() << "\n";
      file << "\n";
    }
  }

  return true;
}

std::string ExtractorAPI::generate_animation_ini_content(const std::string& art_file_path) const {
  std::ostringstream content;

  // Write header for this ART file
  content << "; Animation data from \"" << art_file_path << "\"\n"
          << "; Extracted by art2img\n"
          << "\n";

  // Get the art file header and tiles
  if (!is_art_loaded() || !art_file_) {
    return content.str();
  }

  const auto& tiles = art_file_->tiles();
  const auto& header = art_file_->header();

  for (uint32_t i = 0; i < tiles.size(); ++i) {
    const auto& tile = tiles[i];

    if (tile.anim_data != 0) {
      // Check if tile has meaningful animation data
      if (tile.anim_frames() != 0 || tile.anim_type() != 0 || tile.anim_speed() != 0) {
        content << "[tile" << std::setw(4) << std::setfill('0') << (i + header.start_tile)
                << ".tga -> tile" << std::setw(4) << std::setfill('0')
                << (i + header.start_tile + tile.anim_frames()) << ".tga]\n";
        content << "   AnimationType=" << get_animation_type_string(tile.anim_type()) << "\n";
        content << "   AnimationSpeed=" << tile.anim_speed() << "\n";
        content << "\n";
      }

      content << "[tile" << std::setw(4) << std::setfill('0') << (i + header.start_tile)
              << ".tga]\n";
      content << "   XCenterOffset=" << static_cast<int>(tile.x_offset()) << "\n";
      content << "   YCenterOffset=" << static_cast<int>(tile.y_offset()) << "\n";
      content << "   OtherFlags=" << tile.other_flags() << "\n";
      content << "\n";
    }
  }

  return content.str();
}

std::string ExtractorAPI::get_animation_type_string(uint32_t anim_type) const {
  switch (anim_type) {
  case 0:
    return "none";
  case 1:
    return "oscillation";
  case 2:
    return "forward";
  case 3:
    return "backward";
  default:
    return "unknown";
  }
}

}  // namespace art2img