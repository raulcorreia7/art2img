#pragma once

#include <vector>

#include <art2img/convert.hpp>
#if __has_include(<span>)
#include <span>
#endif

namespace art2img::convert::detail {

class ColumnMajorRowRangeOwner {
 public:
  explicit ColumnMajorRowRangeOwner(const TileView& tile)
      : scratch_(tile.width), range_(tile, std::span<types::u8>(scratch_)) {}

  ColumnMajorRowRange& get() noexcept { return range_; }

  const ColumnMajorRowRange& get() const noexcept { return range_; }

  auto begin() noexcept { return range_.begin(); }

  auto end() noexcept { return range_.end(); }

  auto begin() const noexcept { return range_.begin(); }

  auto end() const noexcept { return range_.end(); }

 private:
  std::vector<types::u8> scratch_;
  ColumnMajorRowRange range_;
};

}  // namespace art2img::convert::detail
