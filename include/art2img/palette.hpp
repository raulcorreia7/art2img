#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "exceptions.hpp"

namespace art2img {

class Palette {
public:
  static constexpr size_t SIZE = 256 * 3;  // 256 colors, 3 components each

  Palette();

  // Non-copyable, movable
  Palette(const Palette&) = delete;
  Palette& operator=(const Palette&) = delete;
  Palette(Palette&&) = default;
  Palette& operator=(Palette&&) = default;

  // File-based operations
  bool load_from_file(const std::filesystem::path& filename);
  void load_build_engine_default();
  void load_blood_default();
  void load_duke3d_default();

  // Memory-based operations
  bool load_from_memory(const uint8_t* data, size_t size);

  // Get palette data in BGR format (for TGA/PNG output)
  std::vector<uint8_t> get_bgr_data() const;

  // Accessors
  const std::vector<uint8_t>& raw_data() const {
    return raw_data_;
  }
  bool is_loaded() const {
    return loaded_;
  }

  // Get specific color component
  uint8_t get_red(size_t index) const;
  uint8_t get_green(size_t index) const;
  uint8_t get_blue(size_t index) const;

private:
  std::vector<uint8_t> raw_data_;
  bool loaded_ = false;

  // Helper function to convert 6-bit component to 8-bit
  static uint8_t scale_component(uint8_t value);
};

}  // namespace art2img
