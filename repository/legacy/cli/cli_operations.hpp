#pragma once

#include <memory>
#include <optional>
#include <string>

#include "art2img/art_file.hpp"
#include "art2img/extractor_api.hpp"
#include "config.hpp"  // Keep our configuration structures

// Result structure for composability
struct ProcessingResult {
  bool success = false;
  std::string error_message;
  size_t processed_count = 0;
  size_t failed_count = 0;
};

struct LoadedArtData {
  bool success = false;
  std::unique_ptr<art2img::ExtractorAPI> extractor;
  std::string error_message;
};

struct TileResult {
  bool success;
  uint32_t tile_index;
  std::string error_message;
  std::string output_path;
};

// Composable functions for processing
LoadedArtData load_art_and_palette_composable(const ProcessingOptions& options,
                                              const std::string& art_file_path);

TileResult process_single_tile_composable(const art2img::ImageView& image_view,
                                          const std::string& output_dir,
                                          const ProcessingOptions& options, uint32_t tile_index);

bool save_image_format(const art2img::ImageView& image_view, const std::string& filepath,
                       const std::string& format, bool fix_transparency);

ProcessingResult process_sequential_impl(const ProcessingOptions& options,
                                         const std::string& art_file_path,
                                         const std::string& output_subdir = "",
                                         bool is_directory_mode = false);

ProcessingResult process_parallel_impl(const ProcessingOptions& options,
                                       const std::string& art_file_path,
                                       const std::string& output_subdir = "",
                                       bool is_directory_mode = false);

ProcessingResult process_with_mode(const ProcessingOptions& options,
                                   const std::string& art_file_path,
                                   const std::string& output_subdir = "",
                                   bool is_directory_mode = false, bool use_parallel = false);

std::optional<std::string> find_palette_file(const std::string& user_path,
                                             const std::string& art_file_path);

bool load_palette_with_fallback(art2img::Palette& palette, const ProcessingOptions& options,
                                const std::string& art_file_path);

bool create_output_directories(const std::string& output_dir);