#include "progress_reporter.hpp"

#include <iostream>
#include <sstream>

namespace art2img::cli {

void report_conversion_error(std::size_t tile_index,
                             const art2img::core::Error& error)
{
  std::cerr << "Failed to convert tile " << tile_index << ": " << error.message
            << "\n";
}

void report_completion_summary(const FileProcessingResult& result,
                               const std::filesystem::path& input_file,
                               const std::filesystem::path& output_dir)
{
  if (result.failures > 0) {
    std::cerr << "Completed with " << result.failures << " failures\n";
  }
  else {
    std::cout << "Converted " << result.total_tiles << " tiles from "
              << input_file.filename().string() << " to " << output_dir.string()
              << "\n";
  }
}

void report_format_error(const std::string& error_message)
{
  std::cerr << error_message << '\n';
}

}  // namespace art2img::cli