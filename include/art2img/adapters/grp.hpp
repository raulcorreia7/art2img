#pragma once

#include <expected>
#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <vector>

#include "../core/error.hpp"

namespace art2img::adapters {

namespace core = ::art2img::core;

struct GrpEntry {
  std::string name;
  std::span<const std::byte> data;
};

struct GrpCatalog {
  std::vector<GrpEntry> entries;

 private:
  std::shared_ptr<std::vector<std::byte>> storage_{};

  friend std::expected<GrpCatalog, core::Error> load_grp(
      std::span<const std::byte>) noexcept;
};

std::expected<GrpCatalog, core::Error> load_grp(
    std::span<const std::byte> blob) noexcept;

std::optional<GrpEntry> find_entry(const GrpCatalog& catalog,
                                   std::string_view name) noexcept;

}  // namespace art2img::adapters
