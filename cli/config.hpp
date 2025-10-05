#pragma once

#include <cstddef>
#include <optional>
#include <string>

enum class OptionTranslationErrorCode {
  None = 0,
  InvalidFormat,
  AnimationConflict,
  PaletteConflict,
};

struct OptionTranslationError {
  OptionTranslationErrorCode code = OptionTranslationErrorCode::None;
  std::string message;
};

struct CliOptions {
  std::string input_path;
  std::string output_dir = ".";
  std::string palette_file;
  std::string format = "png";
  bool fix_transparency = true;
  bool quiet = false;
  bool no_anim = false;
  bool merge_anim = false;
  bool enable_parallel = true;
  std::size_t max_threads = 0;
};

struct ProcessingOptions {
  std::string palette_file;
  std::string output_dir;
  std::string format = "png";
  bool fix_transparency = true;
  bool verbose = false;
  bool dump_animation = true;
  bool merge_animation_data = false;
  bool enable_parallel = true;
  std::size_t max_threads = 0;
};

struct OptionTranslationResult {
  std::optional<ProcessingOptions> options;
  std::optional<OptionTranslationError> error;

  [[nodiscard]] bool success() const {
    return options.has_value() && !error.has_value();
  }
};

OptionTranslationResult translate_to_processing_options(const CliOptions& cli_options);
