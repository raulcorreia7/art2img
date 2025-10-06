#define DOCTEST_CONFIG_SUPER_FAST_ASSERTS
#include <doctest/doctest.h>

#include <algorithm>
#include <array>
#include <cstdio>
#include <filesystem>
#include <string>
#include <string_view>
#include <system_error>
#include <vector>

#include "art_file.hpp"
#include "extractor_api.hpp"
#include "palette.hpp"
#include "test_helpers.hpp"

namespace {

std::vector<std::filesystem::path> collect_art_files() {
  auto root = art2img::tests::assets_root();
  std::vector<std::filesystem::path> files;
  for (const auto& entry : std::filesystem::directory_iterator(root)) {
    if (entry.is_regular_file() && entry.path().extension() == ".ART") {
      files.push_back(entry.path());
    }
  }
  std::sort(files.begin(), files.end());
  return files;
}

std::vector<uint32_t> sample_indices(uint32_t count) {
  if (count == 0) {
    return {};
  }
  if (count == 1) {
    return {0};
  }

  std::vector<uint32_t> samples{0, count - 1};
  if (count > 2) {
    samples.push_back(count / 2);
  }
  std::sort(samples.begin(), samples.end());
  samples.erase(std::unique(samples.begin(), samples.end()), samples.end());
  return samples;
}

std::string run_help_command(const std::filesystem::path& binary) {
  const auto command = (binary.string() + " --help");

#if defined(_WIN32)
  FILE* pipe = _popen(command.c_str(), "r");
#else
  FILE* pipe = popen(command.c_str(), "r");
#endif
  if (!pipe) {
    return {};
  }

  std::string output;
  std::array<char, 256> buffer{};
  while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe)) {
    output.append(buffer.data());
  }

#if defined(_WIN32)
  _pclose(pipe);
#else
  pclose(pipe);
#endif
  return output;
}

std::vector<std::filesystem::path> find_binaries(std::string_view stem) {
  const auto bin_dir = std::filesystem::path("bin");
  std::vector<std::filesystem::path> matches;

  if (!std::filesystem::exists(bin_dir)) {
    INFO("bin directory missing: " << std::filesystem::absolute(bin_dir).string());
    return matches;
  }

  std::error_code ec;
  std::filesystem::recursive_directory_iterator it(
      bin_dir, std::filesystem::directory_options::follow_directory_symlink, ec);
  if (ec) {
    INFO("Failed to iterate bin directory: " << ec.message());
    return matches;
  }

  for (; it != std::filesystem::recursive_directory_iterator(); it.increment(ec)) {
    if (ec) {
      INFO("Iteration error: " << ec.message());
      ec.clear();
      continue;
    }

    const auto& entry = *it;
    if (!entry.is_regular_file(ec)) {
      ec.clear();
      continue;
    }

    const auto& path = entry.path();
    const auto extension = path.extension().string();
    if (path.stem() == stem && (extension.empty() || extension == ".exe")) {
      matches.push_back(path);
    }
  }

  return matches;
}

}  // namespace

TEST_CASE("ArtFile caches data for zero-copy access when loaded from disk") {
  auto art_path = test_asset_path("TILES000.ART");
  art2img::ArtFile art_file(art_path);

  CHECK(art_file.is_open());
  CHECK(art_file.has_data());
  CHECK_NE(art_file.data(), nullptr);
  CHECK_GT(art_file.data_size(), 0);

  const auto& tiles = art_file.tiles();
  REQUIRE_FALSE(tiles.empty());
  CHECK_NE(tiles.front().offset, 0U);
}

TEST_CASE("Default Duke3D palette matches external PALETTE.DAT output") {
  const auto art_files = collect_art_files();
  REQUIRE_FALSE(art_files.empty());

  const auto palette_bytes = load_test_asset("PALETTE.DAT");
  REQUIRE_GE(palette_bytes.size(), art2img::Palette::SIZE);

  size_t compared_tiles = 0;

  for (const auto& art_path : art_files) {
    const auto art_bytes = load_test_asset(art_path.filename().string());

    art2img::ExtractorAPI default_extractor;
    REQUIRE(default_extractor.load_art_from_memory(art_bytes.data(), art_bytes.size()));
    default_extractor.set_duke3d_default_palette();

    art2img::ExtractorAPI explicit_extractor;
    REQUIRE(explicit_extractor.load_art_from_memory(art_bytes.data(), art_bytes.size()));
    REQUIRE(
        explicit_extractor.load_palette_from_memory(palette_bytes.data(), art2img::Palette::SIZE));

    const auto tile_count = default_extractor.get_tile_count();
    REQUIRE_EQ(tile_count, explicit_extractor.get_tile_count());
    REQUIRE_GT(tile_count, 0U);

    for (auto tile_index : sample_indices(tile_count)) {
      auto default_tile = default_extractor.extract_tile_png(tile_index);
      auto explicit_tile = explicit_extractor.extract_tile_png(tile_index);

      CHECK_EQ(default_tile.success, explicit_tile.success);
      CHECK_EQ(default_tile.width, explicit_tile.width);
      CHECK_EQ(default_tile.height, explicit_tile.height);
      CHECK_EQ(default_tile.format, explicit_tile.format);
      CHECK_EQ(default_tile.anim_frames, explicit_tile.anim_frames);
      CHECK_EQ(default_tile.anim_type, explicit_tile.anim_type);
      CHECK_EQ(default_tile.anim_speed, explicit_tile.anim_speed);
      CHECK_EQ(default_tile.other_flags, explicit_tile.other_flags);

      if (default_tile.success) {
        CHECK_EQ(default_tile.image_data, explicit_tile.image_data);
      }

      ++compared_tiles;
    }
  }

  CHECK_GT(compared_tiles, 0U);
}

TEST_CASE("Extractor produces both PNG and TGA payloads in memory") {
  const auto art_bytes = load_test_asset("TILES000.ART");
  const auto palette_bytes = load_test_asset("PALETTE.DAT");

  art2img::ExtractorAPI extractor;
  REQUIRE(extractor.load_art_from_memory(art_bytes.data(), art_bytes.size()));
  REQUIRE(extractor.load_palette_from_memory(palette_bytes.data(), art2img::Palette::SIZE));

  auto png_result = extractor.extract_tile_png(0);
  REQUIRE(png_result.success);
  REQUIRE_FALSE(png_result.image_data.empty());
  CHECK_GT(png_result.width, 0);
  CHECK_GT(png_result.height, 0);
  CHECK_EQ(png_result.format, "png");
  CHECK_GE(png_result.image_data.size(), 8U);
  constexpr std::array<unsigned char, 8> png_magic{{137, 80, 78, 71, 13, 10, 26, 10}};
  CHECK(std::equal(png_magic.begin(), png_magic.end(), png_result.image_data.begin()));

  auto tga_result = extractor.extract_tile_tga(0);
  REQUIRE(tga_result.success);
  REQUIRE_FALSE(tga_result.image_data.empty());
  CHECK_EQ(tga_result.format, "tga");
}

TEST_CASE("Palette scaling preserves 6-bit components and Build-style BGR output") {
  art2img::Palette palette;
  palette.load_duke3d_default();

  const auto& raw = palette.raw_data();
  const auto& bgr = palette.get_bgr_data();

  REQUIRE_EQ(raw.size(), art2img::Palette::SIZE);
  REQUIRE_EQ(bgr.size(), art2img::Palette::SIZE);

  for (size_t i = 0; i < art2img::Palette::SIZE; i += 3) {
    const uint8_t r = raw[i + 0];
    const uint8_t g = raw[i + 1];
    const uint8_t b = raw[i + 2];

    CHECK_LE(r, 63);
    CHECK_LE(g, 63);
    CHECK_LE(b, 63);

    const auto expand = [](uint8_t value) {
      return static_cast<uint8_t>(std::min<uint16_t>(255, static_cast<uint16_t>(value) << 2));
    };

    CHECK_EQ(bgr[i + 0], expand(b));
    CHECK_EQ(bgr[i + 1], expand(g));
    CHECK_EQ(bgr[i + 2], expand(r));
  }
}

TEST_CASE("CLI help documents Duke Nukem 3D palette usage") {
  const auto binaries = find_binaries("art2img");
  if (binaries.empty()) {
    INFO("No art2img binary located under bin/ - verify build completed successfully");
  } else {
    for (const auto& candidate : binaries) {
      INFO("CLI candidate path: " << candidate.string());
    }
  }

  REQUIRE_FALSE(binaries.empty());
  const auto& binary = binaries.front();

  const auto help_text = run_help_command(binary);
  REQUIRE_FALSE(help_text.empty());
  CHECK(help_text.find("built-in Duke Nukem 3D palette") != std::string::npos);
  CHECK(help_text.find("--palette") != std::string::npos);
}
