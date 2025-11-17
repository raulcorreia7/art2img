#pragma once

#include <cstdint>
#include <expected>
#include <optional>
#include <span>

#include "art.hpp"
#include "color_helpers.hpp"
#include "image.hpp"
#include "palette.hpp"

namespace art2img::core {

// ============================================================================
// CONFIGURATION STRUCTURES
// ============================================================================

struct ConversionOptions {
  bool apply_lookup = false;
  std::optional<std::uint8_t> shade_index{};
  bool fix_transparency = true;
  bool premultiply_alpha = false;
  bool matte_hygiene = false;
};

struct PostprocessOptions {
  bool apply_transparency_fix = true;
  bool premultiply_alpha = false;
  bool sanitize_matte = false;
};

// ============================================================================
// PIPELINE STAGES
// ============================================================================

/**
 * @brief Color sampling stage - convert palette indices to RGBA pixels
 */
class ColorSampler {
 public:
  explicit ColorSampler(PaletteView palette, ConversionOptions options)
      : palette_(std::move(palette)), options_(std::move(options))
  {
  }

  /**
   * @brief Sample color for a single palette index
   * @param index Palette index
   * @param lookup Lookup table data (if available)
   * @return RGBA pixel with expanded color and transparency applied
   */
  RgbaPixel sample_color(std::uint8_t index,
                         std::span<const std::byte> lookup = {}) const noexcept;

 private:
  PaletteView palette_;
  ConversionOptions options_;

  std::uint8_t apply_lookup(std::uint8_t index,
                            std::span<const std::byte> lookup) const noexcept;
  std::uint8_t apply_shade(std::uint8_t index) const noexcept;
};

/**
 * @brief Pixel transformation stage - apply post-processing effects
 */
class PixelTransformer {
 public:
  explicit PixelTransformer(PostprocessOptions options)
      : options_(std::move(options))
  {
  }

  /**
   * @brief Transform RGBA image with post-processing effects
   * @param image Image to transform (modified in-place)
   */
  void transform(RgbaImage& image) const noexcept;

 private:
  PostprocessOptions options_;
};

// ============================================================================
// MAIN CONVERSION INTERFACE
// ============================================================================

/**
 * @brief Convert tile to RGBA using composable pipeline stages
 * @param tile Source tile data
 * @param palette Color palette
 * @param options Conversion options
 * @return RGBA image or error
 */
std::expected<RgbaImage, Error> palette_to_rgba(const TileView& tile,
                                                PaletteView palette,
                                                ConversionOptions options = {});

/**
 * @brief Legacy compatibility function - converts using monolithic approach
 * @param tile Source tile data
 * @param palette Color palette
 * @param options Conversion options
 * @return RGBA image or error
 */
std::expected<RgbaImage, Error> palette_to_rgba_legacy(
    const TileView& tile,
    PaletteView palette,
    ConversionOptions options = {});

/**
 * @brief Post-process RGBA image
 * @param image Image to process (modified in-place)
 * @param options Post-processing options
 */
void postprocess_rgba(RgbaImage& image, PostprocessOptions options = {});

}  // namespace art2img::core
