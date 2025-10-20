#pragma once

#include <art2img/types.hpp>
#include <span>

namespace art2img::detail {

/// @brief Read a 16-bit little-endian value from a byte span
/// @param data The byte span to read from
/// @param offset The offset in bytes to start reading
/// @return The 16-bit value read from the data, or 0 if offset is out of bounds
inline types::u16 read_u16_le(std::span<const types::byte> data,
                              std::size_t offset) noexcept {
  if (offset + 1 >= data.size()) {
    return 0;
  }
  return static_cast<types::u16>(static_cast<types::u8>(data[offset])) |
         (static_cast<types::u16>(static_cast<types::u8>(data[offset + 1]))
          << 8);
}

/// @brief Read a 32-bit little-endian value from a byte span
/// @param data The byte span to read from
/// @param offset The offset in bytes to start reading
/// @return The 32-bit value read from the data, or 0 if offset is out of bounds
inline types::u32 read_u32_le(std::span<const types::byte> data,
                              std::size_t offset) noexcept {
  if (offset + 3 >= data.size()) {
    return 0;
  }
  return static_cast<types::u32>(static_cast<types::u8>(data[offset])) |
         (static_cast<types::u32>(static_cast<types::u8>(data[offset + 1]))
          << 8) |
         (static_cast<types::u32>(static_cast<types::u8>(data[offset + 2]))
          << 16) |
         (static_cast<types::u32>(static_cast<types::u8>(data[offset + 3]))
          << 24);
}

}  // namespace art2img::detail