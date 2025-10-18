#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include "exceptions.hpp"

namespace art2img {

class ArtFile {
public:
  struct Header {
    uint32_t version;
    uint32_t start_tile;
    uint32_t end_tile;
    uint32_t num_tiles;

    constexpr bool is_valid() const {
      return version == 1 && end_tile >= start_tile;
    }
  };

  struct Tile {
    uint16_t width;
    uint16_t height;
    uint32_t anim_data;
    uint32_t offset;

    constexpr uint32_t size() const {
      return static_cast<uint32_t>(width) * height;
    }
    constexpr bool is_empty() const {
      return size() == 0;
    }

    // Animation data accessors
    constexpr uint32_t anim_frames() const {
      return anim_data & 0x3F;
    }
    constexpr uint32_t anim_type() const {
      return (anim_data >> 6) & 0x03;
    }
    constexpr int8_t x_offset() const {
      return static_cast<int8_t>((anim_data >> 8) & 0xFF);
    }
    constexpr int8_t y_offset() const {
      return static_cast<int8_t>((anim_data >> 16) & 0xFF);
    }
    constexpr uint32_t anim_speed() const {
      return (anim_data >> 24) & 0x0F;
    }
    constexpr uint32_t other_flags() const {
      return anim_data >> 28;
    }
  };

  ArtFile() : header_{} {}
  explicit ArtFile(const std::filesystem::path& filename);
  explicit ArtFile(const uint8_t* data, size_t size);

  // Non-copyable, movable
  ArtFile(const ArtFile&) = delete;
  ArtFile& operator=(const ArtFile&) = delete;
  ArtFile(ArtFile&&) = default;
  ArtFile& operator=(ArtFile&&) = default;

  // Unified loading operations
  bool load(const std::filesystem::path& filename);
  bool load(const uint8_t* data, size_t size);
  void close();

  bool read_header();
  bool read_tile_metadata();
  bool read_tile_data(uint32_t index, std::vector<uint8_t>& buffer);

  // Accessors
  const Header& header() const {
    return header_;
  }
  const std::vector<Tile>& tiles() const {
    return tiles_;
  }
  bool is_open() const {
    return file_.is_open() || !data_.empty();
  }
  const std::filesystem::path& filename() const {
    return filename_;
  }

  // Memory data access for zero-copy operations
  const uint8_t* data() const {
    return data_.data();
  }
  size_t data_size() const {
    return data_.size();
  }
  bool has_data() const {
    return !data_.empty();
  }

private:
  bool read_header_from_memory();
  bool read_tile_metadata_from_memory();
  bool calculate_offsets();

  uint16_t read_little_endian_uint16(std::ifstream& file);
  uint32_t read_little_endian_uint32(std::ifstream& file);

  uint16_t read_little_endian_uint16_from_memory(size_t& offset) const;
  uint32_t read_little_endian_uint32_from_memory(size_t& offset) const;

  std::filesystem::path filename_;
  std::ifstream file_;
  std::vector<uint8_t> data_;  // For memory-based operations
  Header header_;
  std::vector<Tile> tiles_;
};

}  // namespace art2img