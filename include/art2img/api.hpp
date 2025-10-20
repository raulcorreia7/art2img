#pragma once

/**
 * @file api.hpp
 * @brief Barrel include for the art2img public API
 *
 * This header provides a single entry point to all art2img functionality.
 * Include this header to access the complete art2img API.
 */

// Core types and error handling
#include "error.hpp"
#include "types.hpp"

// Core modules
#include "art.hpp"
#include "convert.hpp"
#include "encode.hpp"
#include "io.hpp"
#include "palette.hpp"

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
  static constexpr int major = 2;
  static constexpr int minor = 0;
  static constexpr int patch = 0;

  /**
   * @brief Get version string
   * @return Version string in format "major.minor.patch"
   */
  static constexpr const char *string() { return "2.0.0"; }
};

} // namespace art2img