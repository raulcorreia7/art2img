#include <art2img/adapters/io.hpp>

#include <climits>
#include <expected>
#include <filesystem>
#include <fstream>
#include <limits>
#include <string>
#include <utility>

namespace art2img::adapters {

std::expected<std::vector<std::byte>, core::Error> read_binary_file(
    const std::filesystem::path& path)
{
  std::ifstream file(path, std::ios::binary | std::ios::ate);
  if (!file) {
    return std::unexpected(core::make_error(
        core::errc::io_failure, "failed to open file: " + path.string()));
  }

  const auto size_pos = file.tellg();
  if (size_pos < 0) {
    return std::unexpected(
        core::make_error(core::errc::io_failure,
                         "failed to determine file size: " + path.string()));
  }

  // Convert file position to size_t safely
  std::size_t size = static_cast<std::size_t>(size_pos);
  if (size_pos != static_cast<decltype(size_pos)>(size)) {
    return std::unexpected(
        core::make_error(core::errc::io_failure,
                         "file too large to allocate: " + path.string()));
  }

  std::vector<std::byte> buffer(static_cast<std::size_t>(size));
  file.seekg(0, std::ios::beg);
  if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
    return std::unexpected(core::make_error(
        core::errc::io_failure, "failed to read file: " + path.string()));
  }

  return buffer;
}

std::expected<void, core::Error> write_file(const std::filesystem::path& path,
                                            std::span<const std::byte> data)
{
  std::ofstream file(path, std::ios::binary | std::ios::trunc);
  if (!file) {
    return std::unexpected(
        core::make_error(core::errc::io_failure,
                         "failed to open file for writing: " + path.string()));
  }

  if (!data.empty() &&
      !file.write(reinterpret_cast<const char*>(data.data()), data.size())) {
    return std::unexpected(core::make_error(
        core::errc::io_failure, "failed to write file: " + path.string()));
  }

  return {};
}

}  // namespace art2img::adapters
