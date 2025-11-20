#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include <art2img/common.hpp>

namespace art2img {

struct GrpEntry {
  std::string name;
  std::vector<Byte> data;
};

struct GrpFile {
  std::vector<GrpEntry> entries;
};

Result<GrpFile> load_grp(const std::filesystem::path& path);

} // namespace art2img
