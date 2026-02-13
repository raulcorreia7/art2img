#pragma once

#include <art2img.hpp>

#include <optional>
#include <string>

namespace art2img_cli {

// ============================================================================
// Configuration Types
// ============================================================================

enum class Mode {
  kConvert,  // Convert ART to images
  kList,     // List GRP contents
  kHelp,     // Show help
  kVersion   // Show version
};

struct Config {
  Mode run_mode = Mode::kConvert;

  // Input options
  std::optional<std::string> grp_path;
  std::optional<std::string> art_path;
  std::optional<std::string> art_name;  // For GRP extraction
  std::optional<std::string> palette_path;

  // Output options
  std::string output_dir = ".";
  art2img::Format fmt = art2img::Format::png;

  // Rendering options
  art2img::RenderOptions render;

  // Behavior
  bool verbose = false;
};

// ============================================================================
// Argument Parsing
// ============================================================================

// Parse command line arguments into config
// Returns: config on success, error message on failure
art2img::Result<Config> parse_args(int argc, char** argv) noexcept;

// Print usage to stderr
void print_usage(const char* prog);

// Print version
void print_version();

}  // namespace art2img_cli
