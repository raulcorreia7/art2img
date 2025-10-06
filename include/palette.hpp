/**
 * @file palette.hpp
 * @brief Palette management for Build engine color handling
 *
 * This file defines the Palette class which provides color palette loading,
 * conversion, and management for Build engine games.
 */

#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

#include "exceptions.hpp"

namespace art2img {

/**
 * @class Palette
 * @brief Represents a color palette used by Build engine games
 *
 * This class manages 256-color palettes with 6-bit color components that are
 * scaled to 8-bit for modern image formats. It supports loading from files,
 * building default palettes for specific games (Duke Nukem 3D, Blood), and
 * converting between color formats.
 */
class Palette {
public:
  /// @brief Size of a complete palette in bytes
  static constexpr size_t SIZE = 256 * 3;  // 256 colors, 3 components each

  /// @brief Default constructor
  Palette();

  // Non-copyable, movable
  /// @brief Deleted copy constructor
  Palette(const Palette&) = delete;
  /// @brief Deleted copy assignment operator
  Palette& operator=(const Palette&) = delete;
  /// @brief Default move constructor
  Palette(Palette&&) = default;
  /// @brief Default move assignment operator
  Palette& operator=(Palette&&) = default;

  // File-based operations
  /// @brief Load palette from a file
  /// @param filename Path to the palette file
  /// @return true if loaded successfully, false otherwise
  bool load_from_file(const std::filesystem::path& filename);

  /// @brief Load the default Duke Nukem 3D palette
  void load_duke3d_default();

  /// @brief Load the default Blood palette
  void load_blood_default();

  // Memory-based operations
  /// @brief Load palette from memory buffer
  /// @param data Pointer to palette data
  /// @param size Size of palette data (must be at least SIZE bytes)
  /// @return true if loaded successfully, false otherwise
  bool load_from_memory(const uint8_t* data, size_t size);

  /// @brief Get palette data in BGR format for image output
  /// @return Vector containing palette data in BGR format (8-bit components)
  std::vector<uint8_t> get_bgr_data() const;

  // Accessors
  /// @brief Get raw palette data (6-bit components)
  /// @return Reference to the raw palette data vector
  const std::vector<uint8_t>& raw_data() const noexcept {
    return raw_data_;
  }

  /// @brief Check if palette is loaded
  /// @return true if palette contains valid data, false otherwise
  bool is_loaded() const noexcept {
    return loaded_;
  }

  // Get specific color component
  /// @brief Get red component for a specific palette index
  /// @param index Color index (0-255)
  /// @return Red component value (0-63, scaled to 0-255)
  uint8_t get_red(size_t index) const;

  /// @brief Get green component for a specific palette index
  /// @param index Color index (0-255)
  /// @return Green component value (0-63, scaled to 0-255)
  uint8_t get_green(size_t index) const;

  /// @brief Get blue component for a specific palette index
  /// @param index Color index (0-255)
  /// @return Blue component value (0-63, scaled to 0-255)
  uint8_t get_blue(size_t index) const;

private:
  std::vector<uint8_t> raw_data_;  ///< Raw palette data (768 bytes, 6-bit components)
  bool loaded_ = false;            ///< Flag indicating if palette is loaded

  // Helper function to convert 6-bit component to 8-bit
  /// @brief Scale 6-bit color component to 8-bit
  /// @param value 6-bit component value (0-63)
  /// @return 8-bit component value (0-255)
  static uint8_t scale_component(uint8_t value);
};

}  // namespace art2img
