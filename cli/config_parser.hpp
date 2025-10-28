#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <optional>
#include <string>
#include <string_view>

#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>

namespace art2img::cli {

struct CliConfig {
  std::filesystem::path input_art;
  std::filesystem::path palette_path;
  std::filesystem::path output_dir{"."};
  std::string format{"png"};
  bool apply_lookup{true};
  bool fix_transparency{true};
  bool premultiply_alpha{false};
  bool sanitize_matte{false};
  std::optional<std::uint8_t> shade_index{};
};

std::expected<art2img::core::ImageFormat, std::string> parse_format(
    std::string_view text);

}  // namespace art2img::cli