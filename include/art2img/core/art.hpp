#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <vector>

#include <art2img/art.hpp>

#include "error.hpp"

namespace art2img::core {

struct TileMetrics {
  std::uint32_t width = 0;
  std::uint32_t height = 0;
};

struct TileView {
  std::span<const std::byte> indices{};
  std::uint32_t width = 0;
  std::uint32_t height = 0;

 private:
  const ::art2img::TileView* legacy_ = nullptr;

  friend std::optional<TileView> get_tile(const ArtArchive&,
                                          std::size_t) noexcept;
  friend const ::art2img::TileView* detail_access(
      const TileView&) noexcept;
};

struct ArtArchive {
  std::span<const std::byte> raw{};
  std::vector<TileMetrics> layout{};

 private:
  std::shared_ptr<const ::art2img::ArtData> backing_{};

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

const ::art2img::TileView* detail_access(const TileView&) noexcept;

}  // namespace art2img::core
