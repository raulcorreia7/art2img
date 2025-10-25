#include <art2img/core/palette.hpp>

#include <algorithm>
#include <cstddef>
#include <expected>
#include <memory>
#include <span>
#include <utility>

#include <art2img/palette.hpp>
#include <art2img/types.hpp>

namespace art2img::core {

std::expected<Palette, Error> load_palette(
    std::span<const std::byte> blob) noexcept {
  auto convert_span = std::span(reinterpret_cast<const ::art2img::types::byte*>(
                                     blob.data()),
                                 blob.size());
  auto parsed = ::art2img::load_palette(convert_span);
  if (!parsed) {
    return std::unexpected(parsed.error());
  }

  auto backing = std::make_shared<::art2img::Palette>(std::move(parsed.value()));
  Palette palette{};
  std::copy(backing->data.begin(), backing->data.end(), palette.rgb.begin());
  palette.backing_ = std::move(backing);
  return palette;
}

PaletteView view_palette(const Palette& palette) noexcept {
  PaletteView view{std::span<const std::uint8_t>(palette.rgb)};
  view.backing_ = palette.backing_;
  return view;
}

std::shared_ptr<const ::art2img::Palette> detail_access(
    const Palette& palette) noexcept {
  return palette.backing_;
}

std::shared_ptr<const ::art2img::Palette> detail_access(
    PaletteView view) noexcept {
  return view.backing_;
}

}  // namespace art2img::core
