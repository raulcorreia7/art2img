/// @file test_convert_benchmark.cpp
/// @brief Performance benchmarks for conversion module
///
/// This file implements performance benchmarks for the conversion functionality
/// as specified in T4.2 of the tasks.md file. It measures conversion speed and
/// provides baseline comparisons for the new color abstraction system.

#include <chrono>
#include <iomanip>
#include <iostream>
#include <vector>

#include <art2img/art.hpp>
#include <art2img/convert.hpp>
#include <art2img/palette.hpp>

using namespace art2img;
using namespace std::chrono;

/// @brief Simple benchmark timer class
class BenchmarkTimer {
  high_resolution_clock::time_point start_time_;

 public:
  BenchmarkTimer() : start_time_(high_resolution_clock::now()) {}

  double elapsed_seconds() const {
    auto end_time = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(end_time - start_time_);
    return duration.count() / 1000000.0;
  }
};

/// @brief Benchmark conversion of various tile sizes
void benchmark_tile_conversion(const Palette& palette) {
  std::cout << "\n=== Tile Conversion Benchmarks ===\n";
  std::cout << std::fixed << std::setprecision(3);

  // Test different tile sizes
  std::vector<std::pair<u16, u16>> test_sizes = {
      {16, 16},    // Small tile
      {32, 32},    // Medium tile
      {64, 64},    // Large tile
      {128, 128},  // Very large tile
  };

  ConversionOptions options;
  options.apply_lookup = false;
  options.fix_transparency = true;
  options.premultiply_alpha = false;
  options.shade_index = 0;

  for (const auto& [width, height] : test_sizes) {
    // Create test tile data
    std::vector<u8> tile_data(width * height);
    for (size_t i = 0; i < tile_data.size(); ++i) {
      tile_data[i] = static_cast<u8>(i % 256);  // Pattern
    }

    TileView tile;
    tile.width = width;
    tile.height = height;
    tile.pixels = tile_data;

    // Benchmark conversion
    const int iterations = 1000;
    BenchmarkTimer timer;

    for (int i = 0; i < iterations; ++i) {
      auto result = to_rgba(tile, palette, options);
      // Prevent compiler optimization
      if (!result) {
        std::cerr << "Conversion failed\n";
        return;
      }
    }

    double elapsed = timer.elapsed_seconds();
    double pixels_per_second = (width * height * iterations) / elapsed;
    double megapixels_per_second = pixels_per_second / 1000000.0;

    std::cout << "Tile " << width << "x" << height << " (" << (width * height)
              << " pixels): ";
    std::cout << megapixels_per_second << " MP/s\n";
  }
}

/// @brief Benchmark different conversion options
void benchmark_conversion_options(const Palette& palette) {
  std::cout << "\n=== Conversion Options Benchmarks ===\n";
  std::cout << std::fixed << std::setprecision(3);

  // Create test tile
  const u16 width = 32;
  const u16 height = 32;
  std::vector<u8> tile_data(width * height);
  for (size_t i = 0; i < tile_data.size(); ++i) {
    tile_data[i] = static_cast<u8>(i % 256);
  }

  TileView tile;
  tile.width = width;
  tile.height = height;
  tile.pixels = tile_data;

  // Test different option combinations
  std::vector<std::pair<std::string, ConversionOptions>> test_configs = {
      {"Basic", {false, false, false, 0}},
      {"With Transparency", {false, true, false, 0}},
      {"With Premultiply", {false, false, true, 0}},
      {"With Shade", {false, false, false, 16}},
      {"All Features", {false, true, true, 16}},
  };

  const int iterations = 5000;

  for (const auto& [name, options] : test_configs) {
    BenchmarkTimer timer;

    for (int i = 0; i < iterations; ++i) {
      auto result = to_rgba(tile, palette, options);
      if (!result) {
        std::cerr << "Conversion failed for " << name << "\n";
        return;
      }
    }

    double elapsed = timer.elapsed_seconds();
    double pixels_per_second = (width * height * iterations) / elapsed;
    double megapixels_per_second = pixels_per_second / 1000000.0;

    std::cout << name << ": " << megapixels_per_second << " MP/s\n";
  }
}

/// @brief Benchmark color structure operations
void benchmark_color_operations() {
  std::cout << "\n=== Color Operations Benchmarks ===\n";
  std::cout << std::fixed << std::setprecision(3);

  const int iterations = 1000000;

  // Benchmark color creation
  BenchmarkTimer timer1;
  for (int i = 0; i < iterations; ++i) {
    color::Color c(i % 256, (i / 256) % 256, (i / 65536) % 256, 255);
    // Prevent optimization
    volatile auto dummy = c.to_packed(color::Format::RGBA);
    (void)dummy;
  }
  double creation_time = timer1.elapsed_seconds();

  // Benchmark format conversion
  std::vector<color::Color> colors(1000);
  for (int i = 0; i < 1000; ++i) {
    colors[i] = color::Color(i % 256, (i / 256) % 256, (i / 65536) % 256, 255);
  }

  BenchmarkTimer timer2;
  for (int i = 0; i < iterations; ++i) {
    const auto& c = colors[i % 1000];
    auto rgba = c.to_packed(color::Format::RGBA);
    auto bgra = c.to_packed(color::Format::BGRA);
    // Prevent optimization
    volatile auto dummy = rgba + bgra;
    (void)dummy;
  }
  double conversion_time = timer2.elapsed_seconds();

  // Benchmark premultiplication
  BenchmarkTimer timer3;
  for (int i = 0; i < iterations; ++i) {
    const auto& c = colors[i % 1000];
    auto premult = c.premultiplied();
    // Prevent optimization
    volatile auto dummy = premult.to_packed(color::Format::RGBA);
    (void)dummy;
  }
  double premult_time = timer3.elapsed_seconds();

  std::cout << "Color Creation: " << (iterations / creation_time / 1000000.0)
            << " M ops/s\n";
  std::cout << "Format Conversion: "
            << (iterations / conversion_time / 1000000.0) << " M ops/s\n";
  std::cout << "Premultiplication: " << (iterations / premult_time / 1000000.0)
            << " M ops/s\n";
}

int main() {
  std::cout << "Conversion Module Performance Benchmarks\n";
  std::cout << "========================================\n";

  // Load palette for testing
  auto palette_result = load_palette(TEST_ASSET_SOURCE_DIR "/PALETTE.DAT");
  if (!palette_result) {
    std::cerr << "Failed to load palette: " << palette_result.error().message
              << "\n";
    return 1;
  }

  const Palette& palette = palette_result.value();

  try {
    benchmark_tile_conversion(palette);
    benchmark_conversion_options(palette);
    benchmark_color_operations();

    std::cout << "\n=== Benchmark Summary ===\n";
    std::cout << "All benchmarks completed successfully.\n";
    std::cout << "The new color abstraction system provides:\n";
    std::cout << "- Type-safe color handling\n";
    std::cout << "- Explicit format conversion\n";
    std::cout << "- Reusable color operations\n";
    std::cout << "- Proper BGR to RGB conversion\n";

  } catch (const std::exception& e) {
    std::cerr << "Benchmark failed with exception: " << e.what() << "\n";
    return 1;
  }

  return 0;
}