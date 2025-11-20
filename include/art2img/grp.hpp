#pragma once

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

Result<GrpFile> parse_grp(ByteSpan data);

} // namespace art2img
