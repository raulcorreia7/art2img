#pragma once

#include <filesystem>
#include <memory>
#include <string>
#include <vector>

#include "art_file.hpp"
#include "exceptions.hpp"
#include "image_writer.hpp"
#include "palette.hpp"

namespace art2img {

// Forward declarations
struct ArtView;
struct ImageView;

struct ExtractionResult {
  bool success;
  std::string error_message;
  std::vector<uint8_t> image_data;
  std::string format;  // "png", "tga", or "bmp"
  uint32_t tile_index;
  uint16_t width;
  uint16_t height;

  // Animation data
  uint32_t anim_frames;
  uint32_t anim_type;
  int8_t x_offset;
  int8_t y_offset;
  uint32_t anim_speed;
  uint32_t other_flags;
};

// Zero-copy view structures for parallel processing
struct ArtView {
  const uint8_t* art_data;           // Pointer to original ART memory
  size_t art_size;                   // Size of ART data
  const Palette* palette;            // Palette for color conversion
  ArtFile::Header header;            // ART file header
  std::vector<ArtFile::Tile> tiles;  // Tile metadata

  // On-demand ImageView creation
  size_t image_count() const {
    return tiles.size();
  }
  const ArtFile::Tile& get_tile(uint32_t tile_index) const {
    if (tile_index >= tiles.size()) {
      throw ArtException("Tile index out of range");
    }
    return tiles[tile_index];
  }

  uint32_t get_start_tile_index() const {
    return header.start_tile;
  }
  uint32_t get_end_tile_index() const {
    return header.end_tile;
  }
  uint32_t get_num_tiles() const {
    return header.num_tiles;
  }
};

struct ImageView {
  const ArtView* parent = nullptr;  // Parent ArtView
  uint32_t tile_index = 0;          // Index of this tile

  // Direct memory access
  const uint8_t* pixel_data() const {
    const auto& tile = require_tile();
    if (tile.is_empty()) {
      return nullptr;
    }
    if (!parent || tile.offset + tile.size() > parent->art_size) {
      throw ArtException("Tile data extends beyond buffer size");
    }
    return parent->art_data + tile.offset;
  }

  uint16_t width() const {
    const auto& tile = require_tile();
    return tile.width;
  }

  uint16_t height() const {
    const auto& tile = require_tile();
    return tile.height;
  }

  size_t size() const {
    const auto& tile = require_tile();
    return static_cast<size_t>(tile.width) * tile.height;
  }

  // Animation data accessors
  uint32_t anim_frames() const {
    return require_tile().anim_frames();
  }
  uint32_t anim_type() const {
    return require_tile().anim_type();
  }
  int8_t x_offset() const {
    return require_tile().x_offset();
  }
  int8_t y_offset() const {
    return require_tile().y_offset();
  }
  uint32_t anim_speed() const {
    return require_tile().anim_speed();
  }
  uint32_t other_flags() const {
    return require_tile().other_flags();
  }

  // Image saving (conversion + write happens here)
  bool save_to_image(const std::filesystem::path& path, ImageFormat format,
                     ImageWriter::Options options = ImageWriter::Options()) const;
  bool save_to_png(const std::filesystem::path& path,
                   ImageWriter::Options options = ImageWriter::Options()) const;
  bool save_to_tga(const std::filesystem::path& path) const;
  bool save_to_bmp(const std::filesystem::path& path,
                   ImageWriter::Options options = ImageWriter::Options()) const;

  // Image extraction to memory
  std::vector<uint8_t> extract_to_image(
      ImageFormat format, ImageWriter::Options options = ImageWriter::Options()) const;
  std::vector<uint8_t> extract_to_png(ImageWriter::Options options = ImageWriter::Options()) const;
  std::vector<uint8_t> extract_to_tga() const;

private:
  const ArtFile::Tile& require_tile() const {
    if (!parent) {
      throw ArtException("Invalid ImageView state");
    }
    return parent->get_tile(tile_index);
  }
};

class ExtractorAPI {
public:
  ExtractorAPI();

  // File-based operations
  bool load_art_file(const std::filesystem::path& filename);
  bool load_palette_file(const std::filesystem::path& filename);

  // Memory-based operations
  bool load_art_from_memory(const uint8_t* data, size_t size);
  bool load_palette_from_memory(const uint8_t* data, size_t size);

  // Set default palettes
  void set_duke3d_default_palette();
  void set_blood_default_palette();

  // Extraction methods
  ExtractionResult extract_tile(uint32_t tile_index, ImageFormat format,
                                ImageWriter::Options options = ImageWriter::Options());
  ExtractionResult extract_tile_png(uint32_t tile_index,
                                    ImageWriter::Options options = ImageWriter::Options());
  ExtractionResult extract_tile_tga(uint32_t tile_index,
                                    ImageWriter::Options options = ImageWriter::Options());
  ExtractionResult extract_tile_bmp(uint32_t tile_index,
                                    ImageWriter::Options options = ImageWriter::Options());

  // Batch extraction
  std::vector<ExtractionResult> extract_all_tiles(
      ImageFormat format, ImageWriter::Options options = ImageWriter::Options());
  std::vector<ExtractionResult> extract_all_tiles_png(
      ImageWriter::Options options = ImageWriter::Options());
  std::vector<ExtractionResult> extract_all_tiles_tga(
      ImageWriter::Options options = ImageWriter::Options());
  std::vector<ExtractionResult> extract_all_tiles_bmp(
      ImageWriter::Options options = ImageWriter::Options());

  // Accessors
  bool is_art_loaded() const {
    return art_file_ != nullptr;
  }
  bool is_palette_loaded() const {
    return palette_ != nullptr;
  }
  uint32_t get_tile_count() const {
    return art_file_ ? static_cast<uint32_t>(art_file_->tiles().size()) : 0;
  }

  // Zero-copy view access for parallel processing
  ArtView get_art_view() const;

  // Animation data handling
  bool write_animation_data(const std::string& art_file_path, const std::string& output_dir) const;
  std::string generate_animation_ini_content(const std::string& art_file_path) const;

private:
  std::unique_ptr<ArtFile> art_file_;
  std::unique_ptr<Palette> palette_;

  // Helper for animation data
  std::string get_animation_type_string(uint32_t anim_type) const;
};

}  // namespace art2img
