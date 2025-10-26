#pragma once

#include <cstddef>
#include <expected>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "../core/error.hpp"

namespace art2img::adapters {

struct GrpEntry {
  std::string name;
  std::span<const std::byte> data;
};

class GrpFile {
 public:
  GrpFile() = default;

  const std::vector<GrpEntry>& entries() const noexcept { return entries_; }

  std::optional<GrpEntry> entry(std::string_view name) const noexcept;

 private:
  std::vector<GrpEntry> entries_;
  std::shared_ptr<std::vector<std::byte>> storage_{};

  friend std::expected<GrpFile, core::Error> load_grp(
      std::span<const std::byte>) noexcept;
};

std::expected<GrpFile, core::Error> load_grp(
    std::span<const std::byte> blob) noexcept;

}  // namespace art2img::adapters
