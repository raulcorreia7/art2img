#pragma once

#include <string>

#include "cli_operations.hpp"
#include "config.hpp"  // Include our new config header

/// Result structure for CLI-visible processing operations
struct CliProcessResult {
  bool success = false;
  std::string error_message;
};
/// Process a single ART file and return the detailed result
ProcessingResult process_single_art_file(const ProcessingOptions& options,
                                         const std::string& art_file_path,
                                         const std::string& output_subdir = "",
                                         bool is_directory_mode = false);

/// Process all ART files in a directory using validated processing options
CliProcessResult process_art_directory(const CliOptions& cli_options,
                                       const ProcessingOptions& options);

/// Wrapper function to call process_single_art_file with command line options
CliProcessResult process_single_art_file_wrapper(const CliOptions& cli_options,
                                                 const ProcessingOptions& options);
