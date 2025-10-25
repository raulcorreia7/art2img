#include <art2img/adapters/io.hpp>

#include <expected>
#include <utility>

#include <art2img/types.hpp>

namespace art2img::adapters {

namespace core = ::art2img::core;

std::expected<std::vector<std::byte>, core::Error> read_binary_file(
    const std::filesystem::path& path) {
  auto result = ::art2img::read_binary_file(path);
  if (!result) {
    return std::unexpected(result.error());
  }
  return std::vector<std::byte>(result->begin(), result->end());
}

std::expected<void, core::Error> write_file(const std::filesystem::path& path,
                                            std::span<const std::byte> data) {
  auto result = ::art2img::write_binary_file(
      path, std::span(reinterpret_cast<const ::art2img::types::byte*>(data.data()),
                      data.size()));
  if (!result) {
    return std::unexpected(result.error());
  }
  return {};
}

}  // namespace art2img::adapters
