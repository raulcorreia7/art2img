#include "config.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <string>
#include <string_view>

namespace {
[[nodiscard]] std::string to_lower_copy(std::string value) {
  std::transform(value.begin(), value.end(), value.begin(),
                 [](unsigned char ch) { return static_cast<char>(std::tolower(ch)); });
  return value;
}

[[nodiscard]] std::string trim_copy(const std::string& value) {
  auto begin = value.begin();
  while (begin != value.end() && std::isspace(static_cast<unsigned char>(*begin))) {
    ++begin;
  }

  auto end = value.end();
  while (end != begin && std::isspace(static_cast<unsigned char>(*(end - 1)))) {
    --end;
  }

  return std::string(begin, end);
}

[[nodiscard]] OptionTranslationResult make_error(OptionTranslationErrorCode code,
                                                 std::string message) {
  OptionTranslationResult result;
  result.error = OptionTranslationError{code, std::move(message)};
  return result;
}
}  // namespace

OptionTranslationResult translate_to_processing_options(const CliOptions& cli_options) {
  static constexpr std::array allowed_formats{"png", "tga", "bmp"};

  const std::string normalized_format = to_lower_copy(cli_options.format);
  const bool format_supported =
      std::any_of(allowed_formats.begin(), allowed_formats.end(),
                  [&](const std::string_view candidate) { return candidate == normalized_format; });

  if (!format_supported) {
    return make_error(OptionTranslationErrorCode::InvalidFormat,
                      "Unsupported output format: " + cli_options.format);
  }

  if (cli_options.merge_anim && cli_options.no_anim) {
    return make_error(OptionTranslationErrorCode::AnimationConflict,
                      "Cannot merge animation data when animation export is disabled");
  }

  const std::string trimmed_palette = trim_copy(cli_options.palette_file);
  if (!cli_options.palette_file.empty() && trimmed_palette.empty()) {
    return make_error(OptionTranslationErrorCode::PaletteConflict,
                      "Palette path cannot be blank when provided");
  }

  ProcessingOptions options;
  options.palette_file = trimmed_palette;
  options.output_dir = cli_options.output_dir.empty() ? std::string{"."} : cli_options.output_dir;
  options.format = normalized_format;
  options.fix_transparency = cli_options.fix_transparency;
  options.verbose = !cli_options.quiet;
  options.dump_animation = !cli_options.no_anim;
  options.merge_animation_data = cli_options.merge_anim;

  OptionTranslationResult result;
  result.options = std::move(options);
  return result;
}
