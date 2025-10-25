#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include "error.hpp"

namespace art2img::core {

struct TileMetrics {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
};

struct TileView {
  std::span<const std::byte> indices{};
  std::span<const std::byte> lookup{};
  std::uint32_t width = 0;
  std::uint32_t height = 0;

  constexpr bool valid() const noexcept {
    const auto required = static_cast<std::size_t>(width) *
                          static_cast<std::size_t>(height);
    return indices.size() >= required;
  }
};

struct ArtArchive {
  std::span<const std::byte> raw{};
  std::vector<TileMetrics> layout{};
  std::uint32_t tile_start = 0;

 private:
  std::shared_ptr<const std::vector<std::byte>> storage_{};
  std::vector<std::size_t> pixel_offsets_{};
  std::vector<std::size_t> lookup_offsets_{};
  std::vector<std::size_t> lookup_sizes_{};
  std::size_t pixel_data_offset_ = 0;
  std::size_t lookup_data_offset_ = 0;

  friend std::expected<ArtArchive, Error> load_art(
      std::span<const std::byte>) noexcept;
  friend std::optional<TileView> get_tile(const ArtArchive&,
                                          std::size_t) noexcept;
};

std::expected<ArtArchive, Error> load_art(
    std::span<const std::byte> blob) noexcept;

std::size_t tile_count(const ArtArchive&) noexcept;

std::optional<TileView> get_tile(const ArtArchive&,
                                 std::size_t tile_index) noexcept;

}  // namespace art2img::core
