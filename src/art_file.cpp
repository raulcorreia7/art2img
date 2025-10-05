#include "art_file.hpp"

#include <cstring>
#include <iostream>
#include <stdexcept>

namespace art2img {

ArtFile::ArtFile(const std::filesystem::path& filename) {
  if (!load(filename)) {
    throw ArtException("Failed to load ART file: " + filename.string());
  }
}

ArtFile::ArtFile(const uint8_t* data, size_t size) {
  if (!load(data, size)) {
    throw ArtException("Failed to load ART data from memory");
  }
}

void ArtFile::close() {
  if (file_.is_open()) {
    file_.close();
  }
  filename_.clear();
  data_.clear();
  header_ = Header{};
  tiles_.clear();
}

bool ArtFile::read_header() {
  if (!file_.is_open()) {
    return false;
  }

  file_.seekg(0, std::ios::beg);

  // Read header (16 bytes)
  uint8_t buffer[16];
  if (!file_.read(reinterpret_cast<char*>(buffer), 16)) {
    std::cerr << "Error: Invalid ART file - not enough header data" << std::endl;
    return false;
  }

  // Extract values from buffer (like original art2tga)
  header_.version = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
  uint32_t num_tiles_from_file =
      buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
  header_.start_tile = buffer[8] | (buffer[9] << 8) | (buffer[10] << 16) | (buffer[11] << 24);
  header_.end_tile = buffer[12] | (buffer[13] << 8) | (buffer[14] << 16) | (buffer[15] << 24);

  // Calculate num_tiles based on start and end tiles (as per test expectations)
  header_.num_tiles = header_.end_tile - header_.start_tile + 1;

  if (!header_.is_valid()) {
    std::cerr << "Error: Invalid ART file - bad version number (" << header_.version
              << ") or tile range" << std::endl;
    return false;
  }

  if (header_.num_tiles > 9216) {  // MAX_NB_TILES from original
    std::cerr << "Error: Too many tiles (" << header_.num_tiles << "), maximum is 9216"
              << std::endl;
    return false;
  }

  return true;
}

bool ArtFile::read_header_from_memory() {
  if (data_.size() < 16) {
    std::cerr << "Error: Invalid ART data - not enough header data" << std::endl;
    return false;
  }

  const uint8_t* buffer = data_.data();

  // Extract values from buffer (like original art2tga)
  header_.version = buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
  uint32_t num_tiles_from_file =
      buffer[4] | (buffer[5] << 8) | (buffer[6] << 16) | (buffer[7] << 24);
  header_.start_tile = buffer[8] | (buffer[9] << 8) | (buffer[10] << 16) | (buffer[11] << 24);
  header_.end_tile = buffer[12] | (buffer[13] << 8) | (buffer[14] << 16) | (buffer[15] << 24);

  // Calculate num_tiles based on start and end tiles (as per test expectations)
  header_.num_tiles = header_.end_tile - header_.start_tile + 1;

  if (!header_.is_valid()) {
    std::cerr << "Error: Invalid ART data - bad version number (" << header_.version
              << ") or tile range" << std::endl;
    return false;
  }

  if (header_.num_tiles > 9216) {  // MAX_NB_TILES from original
    std::cerr << "Error: Too many tiles (" << header_.num_tiles << "), maximum is 9216"
              << std::endl;
    return false;
  }

  return true;
}

bool ArtFile::read_tile_metadata() {
  if (!file_.is_open()) {
    return false;
  }

  tiles_.resize(header_.num_tiles);

  // Read tile widths
  file_.seekg(16, std::ios::beg);
  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].width = read_little_endian_uint16(file_);
  }

  // Read tile heights
  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].height = read_little_endian_uint16(file_);
  }

  // Read animation data
  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].anim_data = read_little_endian_uint32(file_);
  }

  return true;
}

bool ArtFile::read_tile_metadata_from_memory() {
  if (data_.size() < 16 + header_.num_tiles * (2 + 2 + 4)) {
    std::cerr << "Error: Invalid ART data - insufficient metadata" << std::endl;
    return false;
  }

  tiles_.resize(header_.num_tiles);

  size_t offset = 16;

  // Read tile widths
  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].width = read_little_endian_uint16_from_memory(offset);
  }

  // Read tile heights
  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].height = read_little_endian_uint16_from_memory(offset);
  }

  // Read animation data
  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].anim_data = read_little_endian_uint32_from_memory(offset);
  }

  return true;
}

bool ArtFile::calculate_offsets() {
  if (tiles_.empty()) {
    return false;
  }

  // Calculate offsets based on header size and metadata
  size_t current_offset = 16 + header_.num_tiles * (2 + 2 + 4);

  for (uint32_t i = 0; i < header_.num_tiles; ++i) {
    tiles_[i].offset = current_offset;
    current_offset += tiles_[i].size();
  }

  return true;
}

bool ArtFile::read_tile_data(uint32_t index, std::vector<uint8_t>& buffer) {
  if (index >= tiles_.size()) {
    std::cerr << "Error: Tile index " << index << " out of range" << std::endl;
    return false;
  }

  const Tile& tile = tiles_[index];

  if (tile.is_empty()) {
    buffer.clear();
    return true;  // Empty tiles are valid
  }

  buffer.resize(tile.size());

  if (file_.is_open()) {
    // File-based reading
    file_.seekg(tile.offset, std::ios::beg);
    if (!file_.read(reinterpret_cast<char*>(buffer.data()), tile.size())) {
      std::cerr << "Error: Cannot read tile " << index << " data" << std::endl;
      return false;
    }
  } else if (!data_.empty() && tile.offset + tile.size() <= data_.size()) {
    // Memory-based reading
    std::memcpy(buffer.data(), data_.data() + tile.offset, tile.size());
  } else {
    std::cerr << "Error: Cannot read tile " << index << " data - invalid offset or size"
              << std::endl;
    return false;
  }

  return true;
}

uint16_t ArtFile::read_little_endian_uint16(std::ifstream& file) {
  uint8_t buffer[2];
  if (!file.read(reinterpret_cast<char*>(buffer), 2)) {
    return 0;
  }
  return static_cast<uint16_t>(buffer[0] | (buffer[1] << 8));
}

uint32_t ArtFile::read_little_endian_uint32(std::ifstream& file) {
  uint8_t buffer[4];
  if (!file.read(reinterpret_cast<char*>(buffer), 4)) {
    return 0;
  }
  return buffer[0] | (buffer[1] << 8) | (buffer[2] << 16) | (buffer[3] << 24);
}

uint16_t ArtFile::read_little_endian_uint16_from_memory(size_t& offset) const {
  if (offset + 2 > data_.size()) {
    return 0;
  }

  uint16_t result = data_[offset] | (data_[offset + 1] << 8);
  offset += 2;
  return result;
}

uint32_t ArtFile::read_little_endian_uint32_from_memory(size_t& offset) const {
  if (offset + 4 > data_.size()) {
    return 0;
  }

  uint32_t result = data_[offset] | (data_[offset + 1] << 8) | (data_[offset + 2] << 16) |
                    (data_[offset + 3] << 24);
  offset += 4;
  return result;
}

bool ArtFile::load(const std::filesystem::path& filename) {
  close();

  // Check if the path is a directory
  if (std::filesystem::is_directory(filename)) {
    throw ArtException("Cannot open file: " + filename.string() + " (is a directory)");
  }

  file_.open(filename, std::ios::binary);
  if (!file_.is_open()) {
    throw ArtException("Cannot open file: " + filename.string());
  }

  filename_ = filename;

  if (!read_header()) {
    close();
    return false;
  }

  if (!read_tile_metadata()) {
    close();
    return false;
  }

  if (!calculate_offsets()) {
    close();
    return false;
  }

  // Cache the entire ART file in memory to support zero-copy operations
  file_.seekg(0, std::ios::end);
  std::streampos file_size = file_.tellg();
  if (file_size <= 0) {
    std::cerr << "Error: Empty ART file '" << filename.string() << "'" << std::endl;
    close();
    return false;
  }

  data_.resize(static_cast<size_t>(file_size));
  file_.seekg(0, std::ios::beg);
  if (!file_.read(reinterpret_cast<char*>(data_.data()), file_size)) {
    std::cerr << "Error: Failed to read ART file into memory: " << filename.string() << std::endl;
    close();
    return false;
  }

  return true;
}

bool ArtFile::load(const uint8_t* data, size_t size) {
  close();

  if (!data || size < 16) {  // Minimum header size
    std::cerr << "Error: Invalid data or insufficient size" << std::endl;
    return false;
  }

  // Copy data to internal buffer
  data_.assign(data, data + size);

  if (!read_header_from_memory()) {
    close();
    return false;
  }

  if (!read_tile_metadata_from_memory()) {
    close();
    return false;
  }

  if (!calculate_offsets()) {
    close();
    return false;
  }

  return true;
}

}  // namespace art2img
