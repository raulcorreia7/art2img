#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <optional>
#include <span>

#include <art2img/palette.hpp>

#include "error.hpp"

namespace art2img::core {

struct PaletteView {
  std::span<const std::uint8_t> rgb{};

 private:
  std::shared_ptr<const ::art2img::Palette> backing_{};

  friend PaletteView view_palette(const Palette&) noexcept;
  friend std::shared_ptr<const ::art2img::Palette> detail_access(
      PaletteView) noexcept;
};

struct Palette {
  std::array<std::uint8_t, 256 * 3> rgb{};

 private:
  std::shared_ptr<const ::art2img::Palette> backing_{};

  friend std::expected<Palette, Error> load_palette(
      std::span<const std::byte>) noexcept;
  friend PaletteView view_palette(const Palette&) noexcept;
  friend std::shared_ptr<const ::art2img::Palette> detail_access(
      const Palette&) noexcept;
};

std::expected<Palette, Error> load_palette(
    std::span<const std::byte> blob) noexcept;

PaletteView view_palette(const Palette&) noexcept;

std::shared_ptr<const ::art2img::Palette> detail_access(
    const Palette&) noexcept;

std::shared_ptr<const ::art2img::Palette> detail_access(
    PaletteView view) noexcept;

}  // namespace art2img::core
