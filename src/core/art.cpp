#include <art2img/core/art.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <expected>
#include <memory>
#include <span>
#include <vector>

namespace art2img::core {
namespace {

constexpr std::size_t kHeaderSize = 16;
constexpr std::size_t kTileWidthBytes = 2;
constexpr std::size_t kTileHeightBytes = 2;
constexpr std::size_t kTileAnimBytes = 4;
constexpr std::size_t kMaxTileCount = 8192;
constexpr std::size_t kLookupStride = 256;

bool checked_advance(std::size_t& offset, std::size_t delta, std::size_t size)
{
  if (offset > size || delta > size - offset) {
    return false;
  }
  offset += delta;
  return true;
}

std::uint16_t read_u16(std::span<const std::byte> data, std::size_t offset)
{
  std::uint16_t value = 0;
  std::memcpy(&value, data.data() + offset, sizeof(value));
  return value;
}

std::uint32_t read_u32(std::span<const std::byte> data, std::size_t offset)
{
  std::uint32_t value = 0;
  std::memcpy(&value, data.data() + offset, sizeof(value));
  return value;
}

bool validate_header(std::uint32_t tile_start,
                     std::uint32_t tile_end,
                     std::size_t tile_count)
{
  if (tile_end < tile_start) {
    return false;
  }
  if (tile_count == 0 || tile_count > kMaxTileCount) {
    return false;
  }
  return true;
}

bool validate_tile_dimensions(std::uint16_t width, std::uint16_t height)
{
  constexpr std::uint16_t kMaxDimension = 4096;
  return width <= kMaxDimension && height <= kMaxDimension;
}

std::size_t safe_pixel_count(std::uint16_t width, std::uint16_t height)
{
  return static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
}

}  // namespace

std::expected<ArtArchive, Error> load_art(
    std::span<const std::byte> blob) noexcept
{
  if (blob.size() < kHeaderSize) {
    return std::unexpected(
        make_error(errc::invalid_art, "ART data too small for header"));
  }

  auto storage =
      std::make_shared<std::vector<std::byte>>(blob.begin(), blob.end());
  std::span<const std::byte> data{storage->data(), storage->size()};

  std::size_t offset = 0;
  const auto version = read_u32(data, offset);
  offset += 4;
  (void)read_u32(data, offset);
  offset += 4;
  const auto tile_start = read_u32(data, offset);
  offset += 4;
  const auto tile_end = read_u32(data, offset);
  offset += 4;

  const auto tile_count = static_cast<std::size_t>(tile_end - tile_start + 1);
  if (version != 1 || !validate_header(tile_start, tile_end, tile_count)) {
    return std::unexpected(make_error(errc::invalid_art, "invalid ART header"));
  }

  const std::size_t arrays_bytes =
      tile_count * (kTileWidthBytes + kTileHeightBytes + kTileAnimBytes);
  if (!checked_advance(offset, arrays_bytes, data.size())) {
    return std::unexpected(
        make_error(errc::invalid_art, "ART data missing tile arrays"));
  }

  // Rewind to parse arrays.
  offset = kHeaderSize;
  std::vector<std::uint16_t> widths(tile_count);
  std::vector<std::uint16_t> heights(tile_count);

  for (std::size_t i = 0; i < tile_count; ++i) {
    widths[i] = read_u16(data, offset);
    offset += kTileWidthBytes;
  }
  for (std::size_t i = 0; i < tile_count; ++i) {
    heights[i] = read_u16(data, offset);
    offset += kTileHeightBytes;
  }
  offset += tile_count * kTileAnimBytes;

  std::size_t total_pixels = 0;
  for (std::size_t i = 0; i < tile_count; ++i) {
    if (!validate_tile_dimensions(widths[i], heights[i])) {
      return std::unexpected(
          make_error(errc::invalid_art, "tile dimensions exceed limits"));
    }
    total_pixels += safe_pixel_count(widths[i], heights[i]);
  }

  const std::size_t pixel_data_offset = offset;
  if (!checked_advance(offset, total_pixels, data.size())) {
    return std::unexpected(
        make_error(errc::invalid_art, "ART data missing pixel payload"));
  }

  ArtArchive archive{};
  archive.storage_ = std::move(storage);
  archive.raw = data;
  archive.tile_start = tile_start;
  archive.pixel_data_offset_ = pixel_data_offset;
  archive.lookup_data_offset_ = offset;
  archive.layout.reserve(tile_count);
  archive.pixel_offsets_.resize(tile_count);
  archive.lookup_offsets_.resize(tile_count);
  archive.lookup_sizes_.resize(tile_count);

  std::size_t pixel_offset = 0;
  for (std::size_t i = 0; i < tile_count; ++i) {
    archive.layout.push_back(TileMetrics{widths[i], heights[i]});
    archive.pixel_offsets_[i] = pixel_offset;
    pixel_offset += safe_pixel_count(widths[i], heights[i]);
  }

  const std::size_t remaining_lookup =
      data.size() > offset ? data.size() - offset : 0;
  if (remaining_lookup == 0) {
    archive.lookup_offsets_.assign(tile_count, 0);
    archive.lookup_sizes_.assign(tile_count, 0);
  }
  else {
    std::size_t lookup_offset = 0;
    for (std::size_t i = 0; i < tile_count; ++i) {
      archive.lookup_offsets_[i] = lookup_offset;
      if (lookup_offset < remaining_lookup) {
        const auto available = remaining_lookup - lookup_offset;
        const auto use = std::min<std::size_t>(kLookupStride, available);
        archive.lookup_sizes_[i] = use;
        lookup_offset += use;
      }
      else {
        archive.lookup_sizes_[i] = 0;
      }
    }
  }

  return archive;
}

std::size_t tile_count(const ArtArchive& archive) noexcept
{
  return archive.layout.size();
}

std::optional<TileView> get_tile(const ArtArchive& archive,
                                 std::size_t tile_index) noexcept
{
  if (tile_index >= archive.layout.size() || !archive.storage_) {
    return std::nullopt;
  }

  const auto& metrics = archive.layout[tile_index];
  const auto required = safe_pixel_count(metrics.width, metrics.height);
  const auto start =
      archive.pixel_data_offset_ + archive.pixel_offsets_[tile_index];
  if (required == 0 || start + required > archive.raw.size()) {
    return std::nullopt;
  }

  TileView view{};
  view.width = metrics.width;
  view.height = metrics.height;
  view.indices = archive.raw.subspan(start, required);

  if (tile_index < archive.lookup_sizes_.size()) {
    const auto lookup_size = archive.lookup_sizes_[tile_index];
    if (lookup_size > 0) {
      const auto lookup_start =
          archive.lookup_data_offset_ + archive.lookup_offsets_[tile_index];
      if (lookup_start + lookup_size <= archive.raw.size()) {
        view.lookup = archive.raw.subspan(lookup_start, lookup_size);
      }
    }
  }

  if (!view.valid()) {
    return std::nullopt;
  }

  return view;
}

}  // namespace art2img::core
