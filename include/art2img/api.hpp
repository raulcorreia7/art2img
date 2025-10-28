#pragma once

/**
 * @file api.hpp
 * @brief Barrel include for the art2img public API
 *
 * This header provides a single entry point to all art2img functionality.
 * Include this header to access the complete art2img API.
 */

// Public memory-first surface
#include <string>
#include <format>
#include "adapters/grp.hpp"
#include "adapters/io.hpp"
#include "adapters/meta_serialization.hpp"
#include "core/art.hpp"
#include "core/convert.hpp"
#include "core/encode.hpp"
#include "core/error.hpp"
#include "core/image.hpp"
#include "core/meta.hpp"
#include "core/palette.hpp"
#include "extras/batch.hpp"
/**
 * @namespace art2img
 * @brief Main namespace for the art2img library
 *
 * art2img provides a complete pipeline for converting ART format files
 * to modern image formats (PNG, TGA, BMP) with support for palettes,
 * transparency, and various conversion options.
 */
namespace art2img {

/**
 * @brief Version information
 */
struct Version {
  static constexpr int major = 1;
  static constexpr int minor = 1;
  static constexpr int patch = 0;

  /**
   * @brief Get version string (compile-time)
   * @return Version string in format "major.minor.patch"
   */
  static constexpr const std::string_view version()
  {
    return std::format("{}.{}.{}", major, minor, patch);
  }
};

}  // namespace art2img
