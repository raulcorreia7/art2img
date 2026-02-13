#pragma once

#include "args.hpp"

#include <art2img.hpp>

namespace art2img_cli {

// ============================================================================
// High-Level Operations
// ============================================================================

// List GRP contents
// Prints to stdout, errors to stderr
// Returns: 0 on success, 1 on error
int cmd_list(const Config& cfg) noexcept;

// Convert ART to images
// Progress to stderr if verbose
// Returns: 0 on success, 1 on error
int cmd_convert(const Config& cfg) noexcept;

// ============================================================================
// Helper Functions
// ============================================================================

// Auto-detect ART name from GRP (first tiles*.art)
std::optional<std::string> auto_detect_art(const art2img::GrpFile& grp) noexcept;

// Build ConvertOptions from config
art2img::ConvertOptions make_convert_opts(const Config& cfg) noexcept;

}  // namespace art2img_cli
