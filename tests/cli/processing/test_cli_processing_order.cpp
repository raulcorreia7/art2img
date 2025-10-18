#include <doctest/doctest.h>

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>
#include <system_error>
#include <vector>

#include "config.hpp"
#include "processor.hpp"
#include "test_helpers.hpp"

namespace {
class TempDirGuard {
public:
  explicit TempDirGuard(std::filesystem::path path) : path_(std::move(path)) {}

  TempDirGuard(const TempDirGuard&) = delete;
  TempDirGuard& operator=(const TempDirGuard&) = delete;

  ~TempDirGuard() {
    if (!path_.empty()) {
      std::error_code ec;
      std::filesystem::remove_all(path_, ec);
    }
  }

  const std::filesystem::path& path() const {
    return path_;
  }

private:
  std::filesystem::path path_;
};

std::filesystem::path make_temp_directory(const std::string& prefix) {
  static std::atomic<uint64_t> counter{0};
  const auto timestamp =
      static_cast<uint64_t>(std::chrono::steady_clock::now().time_since_epoch().count());
  auto base = std::filesystem::temp_directory_path();
  auto temp_dir = base / (prefix + "-" + std::to_string(timestamp) + "-" +
                          std::to_string(counter.fetch_add(1, std::memory_order_relaxed)));
  std::filesystem::create_directories(temp_dir);
  return temp_dir;
}

}  // namespace

TEST_CASE("process_art_directory sorts matching files before processing") {
  const auto palette_path = test_asset_path("PALETTE.DAT");
  REQUIRE(std::filesystem::exists(palette_path));

  const TempDirGuard input_guard(make_temp_directory("art2img-order-input"));
  const TempDirGuard output_guard(make_temp_directory("art2img-order-output"));

  const auto art_source = test_asset_path("TILES000.ART");
  REQUIRE(std::filesystem::exists(art_source));

  std::vector<std::string> filenames = {"zeta.art", "ALPHA.ART", "beta.art"};
  std::vector<std::string> sorted_paths;

  for (const auto& name : filenames) {
    const auto target = input_guard.path() / name;
    std::filesystem::copy_file(art_source, target,
                               std::filesystem::copy_options::overwrite_existing);
    REQUIRE(std::filesystem::exists(target));
    REQUIRE_EQ(std::filesystem::file_size(target), std::filesystem::file_size(art_source));
    sorted_paths.push_back(target.string());
  }

  std::sort(sorted_paths.begin(), sorted_paths.end());

  CliOptions cli_options;
  cli_options.input_path = input_guard.path().string();
  cli_options.output_dir = output_guard.path().string();
  cli_options.format = "png";
  cli_options.fix_transparency = true;
  cli_options.quiet = true;
  cli_options.no_anim = false;
  cli_options.merge_anim = true;

  const auto translation = translate_to_processing_options(cli_options);
  REQUIRE(translation.success());

  const auto result = process_art_directory(cli_options, *translation.options);

  CAPTURE(result.error_message);
  CHECK_MESSAGE(result.success, "Directory processing should succeed: " << result.error_message);

  const auto anim_file = output_guard.path() / "animdata.ini";
  REQUIRE_MESSAGE(std::filesystem::exists(anim_file),
                  "Animation data file not created at " << anim_file.string());

  std::ifstream anim_stream(anim_file);
  REQUIRE(anim_stream.is_open());

  std::vector<std::string> processed_paths;
  std::string line;
  const std::string header_prefix = "; Animation data from \"";
  while (std::getline(anim_stream, line)) {
    if (line.rfind(header_prefix, 0) == 0) {
      const auto start = header_prefix.size();
      const auto end = line.rfind('"');
      if (end != std::string::npos && end > start) {
        processed_paths.emplace_back(line.substr(start, end - start));
      }
    }
  }

  CAPTURE(processed_paths);
  REQUIRE_EQ(processed_paths.size(), sorted_paths.size());
  CHECK_EQ(processed_paths, sorted_paths);
}
