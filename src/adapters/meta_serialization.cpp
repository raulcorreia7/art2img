#include <art2img/adapters/meta_serialization.hpp>

#include <expected>
#include <iomanip>
#include <ostream>
#include <sstream>
#include <utility>

namespace art2img::adapters {

namespace {
core::Error manifest_error(std::string message) {
  return core::make_error(core::errc::conversion_failure, std::move(message));
}

void append_animation_ini(std::ostream& out,
                          const core::AnimationData& animation) {
  out << "[" << animation.name << "]\n";
  out << "frames=" << animation.frames.first << ","
      << animation.frames.count << "\n";
  out << "frame_time_ms=" << animation.frame_time.count() << "\n";
  out << "loops=" << (animation.loops ? "true" : "false") << "\n\n";
}

void append_animation_json(std::ostream& out,
                           const core::AnimationData& animation,
                           bool last) {
  out << "    {\n";
  out << "      \"name\": \"" << animation.name << "\",\n";
  out << "      \"first_frame\": " << animation.frames.first << ",\n";
  out << "      \"frame_count\": " << animation.frames.count << ",\n";
  out << "      \"frame_time_ms\": " << animation.frame_time.count() << ",\n";
  out << "      \"loops\": " << (animation.loops ? "true" : "false") << "\n";
  out << "    }" << (last ? "\n" : ",\n");
}

}  // namespace

std::expected<std::string, core::Error> format_animation_ini(
    const core::ExportManifest& manifest) {
  if (manifest.palette_name.empty()) {
    return std::unexpected(
        manifest_error("manifest requires palette_name"));
  }

  std::ostringstream out;
  out << "; art2img animation manifest\n";
  out << "palette=" << manifest.palette_name << "\n\n";
  for (const auto& animation : manifest.animations) {
    if (animation.name.empty()) {
      return std::unexpected(
          manifest_error("animation name must not be empty"));
    }
    append_animation_ini(out, animation);
  }
  return out.str();
}

std::expected<std::string, core::Error> format_animation_json(
    const core::ExportManifest& manifest) {
  if (manifest.palette_name.empty()) {
    return std::unexpected(
        manifest_error("manifest requires palette_name"));
  }

  std::ostringstream out;
  out << "{\n";
  out << "  \"palette\": \"" << manifest.palette_name << "\",\n";
  out << "  \"animations\": [\n";
  for (std::size_t i = 0; i < manifest.animations.size(); ++i) {
    const auto& animation = manifest.animations[i];
    if (animation.name.empty()) {
      return std::unexpected(
          manifest_error("animation name must not be empty"));
    }
    append_animation_json(out, animation, i + 1 == manifest.animations.size());
  }
  out << "  ]\n";
  out << "}\n";
  return out.str();
}

}  // namespace art2img::adapters
