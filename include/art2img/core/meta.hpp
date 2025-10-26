#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <vector>

namespace art2img::core {

struct FrameRange {
  std::uint32_t first = 0;
  std::uint32_t count = 0;
};

struct AnimationData {
  std::string name;
  FrameRange frames{};
  std::chrono::milliseconds frame_time{0};
  bool loops = true;
};

struct ExportManifest {
  std::string palette_name;
  std::vector<AnimationData> animations;
};

}  // namespace art2img::core
