#pragma once

#include <string>

#include "config.hpp"  // Include our new config header

/// Process a single ART file
bool process_single_art_file(const ProcessingOptions& options, const std::string& art_file_path,
                             const std::string& output_subdir = "", bool is_directory_mode = false);

/// Process all ART files in a directory
bool process_art_directory(const CliOptions& cli_options);

/// Wrapper function to call process_single_art_file with command line options
bool process_single_art_file_wrapper(const CliOptions& cli_options);