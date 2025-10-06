#include <doctest/doctest.h>

#include <atomic>
#include <chrono>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <string>
#include <system_error>

#include "cli_operations.hpp"
#include "palette.hpp"
#include "test_helpers.hpp"

namespace {
class StreamRedirect {
public:
  explicit StreamRedirect(std::ostream& stream)
      : stream_(stream), original_buf_(stream.rdbuf(buffer_.rdbuf())) {}

  StreamRedirect(const StreamRedirect&) = delete;
  StreamRedirect& operator=(const StreamRedirect&) = delete;

  ~StreamRedirect() {
    stream_.rdbuf(original_buf_);
  }

  [[nodiscard]] std::string str() const {
    return buffer_.str();
  }

private:
  std::ostream& stream_;
  std::streambuf* original_buf_;
  std::ostringstream buffer_;
};

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

TEST_CASE("load_palette_with_fallback warns when palette file cannot be located") {
  ProcessingOptions options;
  options.palette_file = "nonexistent_palette.dat";
  options.verbose = true;

  art2img::Palette palette;
  StreamRedirect capture(std::cout);
  CHECK(load_palette_with_fallback(palette, options, "missing_dir/example.art"));
  const auto output = capture.str();

  CHECK_NE(output.find("Warning: Cannot locate palette file 'nonexistent_palette.dat'"),
           std::string::npos);
  CHECK_NE(output.find("Using built-in Duke Nukem 3D palette (256 colors)"), std::string::npos);
}

TEST_CASE("load_palette_with_fallback informs when palette path is omitted") {
  ProcessingOptions options;
  options.verbose = true;

  art2img::Palette palette;
  StreamRedirect capture(std::cout);
  CHECK(load_palette_with_fallback(palette, options, "missing_dir/example.art"));
  const auto output = capture.str();

  CHECK_NE(output.find("Info: No palette file specified, using default Duke Nukem 3D palette"),
           std::string::npos);
  CHECK_NE(output.find("Using built-in Duke Nukem 3D palette (256 colors)"), std::string::npos);
}

TEST_CASE("load_palette_with_fallback surfaces palette loading exceptions before fallback") {
  const TempDirGuard temp_guard(make_temp_directory("art2img-pal-dir"));
  const auto palette_dir = temp_guard.path() / "palette";
  std::filesystem::create_directories(palette_dir);

  ProcessingOptions options;
  options.palette_file = palette_dir.string();
  options.verbose = true;

  art2img::Palette palette;
  StreamRedirect capture(std::cout);
  CHECK(load_palette_with_fallback(palette, options, test_asset_path("TILES000.ART").string()));
  const auto output = capture.str();

  CHECK_NE(output.find("Warning: Cannot open palette file: " + palette_dir.string() +
                       " (is a directory)"),
           std::string::npos);
  CHECK_NE(output.find("Falling back to default palette..."), std::string::npos);
  CHECK_NE(output.find("Warning: Cannot open palette file '" + palette_dir.string() + "'"),
           std::string::npos);
}

TEST_CASE("process_sequential_impl warns when animation data output fails") {
  const auto palette_path = test_asset_path("PALETTE.DAT");
  const auto art_path = test_asset_path("TILES000.ART");
  REQUIRE(std::filesystem::exists(palette_path));
  REQUIRE(std::filesystem::exists(art_path));

  TempDirGuard temp_guard(make_temp_directory("art2img-animdir"));
  const auto& temp_dir = temp_guard.path();
  // Block creation of the animdata.ini file by creating a directory with that name.
  std::filesystem::create_directories(temp_dir / "animdata.ini");

  ProcessingOptions options;
  options.palette_file = palette_path.string();
  options.output_dir = temp_dir.string();
  options.format = "png";
  options.fix_transparency = true;
  options.verbose = false;
  options.dump_animation = true;
  options.merge_animation_data = false;

  StreamRedirect capture(std::cerr);
  auto result = process_sequential_impl(options, art_path.string());
  const auto errors = capture.str();

  CHECK(result.processed_count > 0);
  CHECK_NE(errors.find("Warning: Failed to write animation data for " + art_path.string()),
           std::string::npos);
}
