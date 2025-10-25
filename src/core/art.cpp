#include <art2img/core/art.hpp>

#include <algorithm>
#include <cstddef>
#include <memory>
#include <span>
#include <utility>

#include <art2img/art.hpp>
#include <art2img/error.hpp>

namespace art2img::core {

namespace {
std::span<const std::byte> as_bytes(::art2img::types::u8_span span) {
  auto bytes = std::as_bytes(span);
  return {bytes.data(), bytes.size()};
}
}  // namespace

std::expected<ArtArchive, Error> load_art(
    std::span<const std::byte> blob) noexcept {
  auto convert_span = std::span(reinterpret_cast<const ::art2img::types::byte*>(
                                     blob.data()),
                                 blob.size());
  auto parsed = ::art2img::load_art_bundle(convert_span);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }

  ArtArchive archive{};
  archive.raw = blob;
  archive.backing_ = std::make_shared<::art2img::ArtData>(std::move(parsed.value()));
  archive.layout.reserve(archive.backing_->tiles.size());
  for (const auto& tile : archive.backing_->tiles) {
    archive.layout.push_back(
        TileMetrics{tile.width, tile.height});
  }

  return archive;
}

std::size_t tile_count(const ArtArchive& archive) noexcept {
  return archive.layout.size();
}

std::optional<TileView> get_tile(const ArtArchive& archive,
                                 std::size_t tile_index) noexcept {
  if (!archive.backing_ || tile_index >= archive.backing_->tiles.size()) {
    return std::nullopt;
  }
  const auto& source_tile = archive.backing_->tiles[tile_index];
  if (!source_tile.is_valid()) {
    return std::nullopt;
  }
  TileView view{as_bytes(source_tile.pixels), source_tile.width,
                source_tile.height};
  view.legacy_ = &source_tile;
  return view;
}

const ::art2img::TileView* detail_access(const TileView& tile) noexcept {
  return tile.legacy_;
}

}  // namespace art2img::core
