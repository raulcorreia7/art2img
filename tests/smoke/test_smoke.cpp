#include <doctest/doctest.h>
#include <chrono>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <string>

#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/palette.hpp>
#include "../test_helpers.hpp"

TEST_SUITE("Smoke Tests")
{
  TEST_CASE("CLI executable exists and is accessible")
  {
    // Test that the CLI executable can be found and executed
    std::string cli_path = test_helpers::get_cli_executable_path().string();

    // First check if the executable exists
    CHECK_MESSAGE(std::filesystem::exists(cli_path),
                  "CLI executable should exist at: " << cli_path);

    // Try to run the CLI with --help flag
    int result = std::system((cli_path + " --help > /dev/null 2>&1").c_str());

    // Exit code 0 means help was displayed successfully
    CHECK_MESSAGE(result == 0,
                  "CLI executable should be accessible and show help");
  }

  TEST_CASE("CLI help command works correctly")
  {
    // Test that CLI shows help when requested
    std::string cli_path = test_helpers::get_cli_executable_path().string();

    // First check if the executable exists
    REQUIRE_MESSAGE(std::filesystem::exists(cli_path),
                    "CLI executable should exist at: " << cli_path);

    std::string cmd = cli_path + " --help";
    FILE* pipe = popen(cmd.c_str(), "r");

    REQUIRE_MESSAGE(pipe != nullptr,
                    "Should be able to execute CLI help command");

    // Read first few lines to verify help content
    char buffer[256];
    bool has_help_content = false;

    while (fgets(buffer, sizeof(buffer), pipe) != nullptr) {
      std::string line(buffer);
      // Look for typical help content indicators
      if (line.find("Convert Build Engine ART tiles") != std::string::npos ||
          line.find("-i,--input") != std::string::npos ||
          line.find("-p,--palette") != std::string::npos ||
          line.find("-o,--output") != std::string::npos) {
        has_help_content = true;
        break;
      }
    }

    pclose(pipe);
    CHECK_MESSAGE(has_help_content,
                  "CLI help should contain expected usage information");
  }
}

TEST_CASE("Core modules can be loaded and instantiated")
{
  // Test that core modules can be included and basic types work
  using art2img::core::ArtArchive;
  using art2img::core::Palette;
  using art2img::core::TileMetrics;
  using art2img::core::TileView;

  // Basic compilation and type existence test
  CHECK_MESSAGE(true, "Core modules should compile and be accessible");

  // Test that we can create basic objects (if constructors are available)
  // This is a minimal smoke test - more comprehensive tests are in unit tests
  SUBCASE("ArtArchive type is available")
  {
    // Just verify the type exists and is complete
    CHECK_MESSAGE(sizeof(ArtArchive) > 0,
                  "ArtArchive should be a complete type");
  }

  SUBCASE("Palette type is available")
  {
    // Just verify the type exists and is complete
    CHECK_MESSAGE(sizeof(Palette) > 0, "Palette should be a complete type");
  }
}

TEST_CASE("Test assets exist and are readable")
{
  const auto asset_dir = test_helpers::get_test_assets_dir();

  // Verify asset directory exists
  CHECK_MESSAGE(std::filesystem::exists(asset_dir),
                "Test asset directory should exist: " << asset_dir.string());

  // Check for essential test files
  std::vector<std::string> essential_files = {"PALETTE.DAT", "LOOKUP.DAT",
                                              "TABLES.DAT", "TILES000.ART"};

  for (const auto& filename : essential_files) {
    auto filepath = asset_dir / filename;
    CHECK_MESSAGE(std::filesystem::exists(filepath),
                  "Essential test file should exist: " << filename);

    // Verify file is readable by opening it
    std::ifstream file(filepath, std::ios::binary);
    CHECK_MESSAGE(file.good(),
                  "Essential test file should be readable: " << filename);

    // Verify file has content (not empty)
    file.seekg(0, std::ios::end);
    auto file_size = file.tellg();
    CHECK_MESSAGE(file_size > 0,
                  "Essential test file should not be empty: " << filename);
  }
}

TEST_CASE("Basic file I/O operations work")
{
  using art2img::adapters::read_binary_file;

  const auto asset_dir = test_helpers::get_test_assets_dir();
  auto palette_path = asset_dir / "PALETTE.DAT";

  // Test that we can read a binary file
  auto result = read_binary_file(palette_path.string());

  CHECK_MESSAGE(result.has_value(), "Should be able to read PALETTE.DAT file");

  if (result.has_value()) {
    const auto& data = result.value();
    CHECK_MESSAGE(!data.empty(), "Read file should contain data");

    // Palette file should have reasonable size (at least 768 bytes for 256
    // colors * 3 channels)
    CHECK_MESSAGE(
        data.size() >= 768,
        "PALETTE.DAT should be at least 768 bytes, got: " << data.size());
  }
}

TEST_CASE("Smoke test execution time is reasonable")
{
  // This test verifies that smoke tests run quickly
  // In practice, this is enforced by the CMake timeout (30 seconds)
  // but we can do a basic timing check here

  auto start = std::chrono::high_resolution_clock::now();

  // Do a quick operation that should be fast
  const auto asset_dir = test_helpers::get_test_assets_dir();
  bool assets_exist = std::filesystem::exists(asset_dir) &&
                      std::filesystem::exists(asset_dir / "PALETTE.DAT");

  auto end = std::chrono::high_resolution_clock::now();
  auto duration =
      std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

  CHECK_MESSAGE(assets_exist, "Assets should exist for timing test");
  CHECK_MESSAGE(duration.count() < 100,
                "Basic operations should complete quickly (<100ms), took: "
                    << duration.count() << "ms");
}

TEST_CASE("Test output directory can be created")
{
  // Test that we can create test output directories
  const auto test_output_dir = std::filesystem::path(TEST_OUTPUT_DIR);

  // Use test helpers to create a directory
  auto smoke_test_dir =
      test_helpers::get_test_output_dir() / "smoke" / "output_test";
  test_helpers::ensure_test_output_dir(smoke_test_dir);

  CHECK_MESSAGE(std::filesystem::exists(smoke_test_dir),
                "Should be able to create smoke test output directory");

  // Test that we can write a file to it
  auto test_file = smoke_test_dir / "test.txt";
  std::ofstream file(test_file);
  file << "smoke test";
  file.close();

  CHECK_MESSAGE(std::filesystem::exists(test_file),
                "Should be able to write files to test output directory");
  CHECK_MESSAGE(std::filesystem::file_size(test_file) > 0,
                "Written file should have content");
}