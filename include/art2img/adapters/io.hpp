#pragma once

#include <cstddef>
#include <cstdint>
#include <expected>
#include <filesystem>
#include <span>
#include <vector>

#include "../core/error.hpp"

namespace art2img::adapters {

std::expected<std::vector<std::byte>, core::Error> read_binary_file(
    const std::filesystem::path& path);

std::expected<void, core::Error> write_file(const std::filesystem::path& path,
                                            std::span<const std::byte> data);

}  // namespace art2img::adapters
