#include "progress_reporter.hpp"

#include <format>
#include <iostream>

namespace art2img::cli {

void report_conversion_error(std::size_t tile_index,
                             const art2img::core::Error& error)
{
  std::cerr << std::format("Failed to convert tile {}: {}\n", tile_index,
                           error.message);
}

void report_completion_summary(const FileProcessingResult& result,
                               const std::filesystem::path& input_file,
                               const std::filesystem::path& output_dir)
{
  if (result.failures > 0) {
    std::cerr << std::format("Completed with {} failures\n", result.failures);
  }
  else {
    std::cout << std::format("Converted {} tiles from {} to {}\n",
                             result.total_tiles, input_file.filename().string(),
                             output_dir.string());
  }
}

void report_format_error(const std::string& error_message)
{
  std::cerr << error_message << '\n';
}

}  // namespace art2img::cli