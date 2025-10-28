#pragma once

#include <cstddef>
#include <filesystem>
#include <string>

#include <art2img/core/error.hpp>

#include "file_processor.hpp"

namespace art2img::cli {

void report_conversion_error(std::size_t tile_index,
                             const art2img::core::Error& error);
void report_completion_summary(const FileProcessingResult& result,
                               const std::filesystem::path& input_file,
                               const std::filesystem::path& output_dir);
void report_format_error(const std::string& error_message);

}  // namespace art2img::cli