#include <doctest/doctest.h>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include "../test_helpers.hpp"

namespace fs = std::filesystem;

// Helper function to get test asset path
fs::path get_test_asset_path(const std::string& filename)
{
  return test_helpers::get_test_assets_dir() / filename;
}

class CLITestFixture {
 public:
  std::string run_cli(const std::vector<std::string>& args)
  {
    std::string cli_path = get_cli_path();
    std::string cmd = cli_path;

    for (const auto& arg : args) {
      cmd += " \"" + arg + "\"";
    }

    std::array<char, 128> buffer;
    std::string result;

    // Capture both stdout and stderr
    std::string full_cmd = cmd + " 2>&1";
    FILE* pipe = popen(full_cmd.c_str(), "r");
    if (!pipe) {
      throw std::runtime_error("popen() failed!");
    }

    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
      result += buffer.data();
    }

    pclose(pipe);
    return result;
  }

  fs::path create_test_dir()
  {
    // Create temporary test directory using test helpers with unique name
    fs::path test_dir = test_helpers::create_unique_test_dir(
        test_helpers::get_test_output_dir() / "integration", "cli_test");

    // Copy test assets to test directory, overwrite if exists
    std::error_code ec;
    fs::copy_file(get_test_asset_path("TILES000.ART"),
                  test_dir / "TILES000.ART",
                  fs::copy_options::overwrite_existing, ec);
    if (ec) {
      throw std::runtime_error("Failed to copy TILES000.ART: " + ec.message());
    }
    fs::copy_file(get_test_asset_path("PALETTE.DAT"), test_dir / "PALETTE.DAT",
                  fs::copy_options::overwrite_existing, ec);
    if (ec) {
      throw std::runtime_error("Failed to copy PALETTE.DAT: " + ec.message());
    }

    return test_dir;
  }

 private:
  fs::path get_cli_path() { return test_helpers::get_cli_executable_path(); }
};

TEST_CASE_FIXTURE(CLITestFixture, "CLI basic conversion")
{
  SUBCASE("Convert ART to PNG with default settings")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    // Check that output files were created
    bool found_png = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".png") {
        found_png = true;
        break;
      }
    }

    CHECK(found_png);
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Convert ART to TGA format")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--format");
    args.push_back("tga");
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    bool found_tga = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".tga") {
        found_tga = true;
        break;
      }
    }

    CHECK(found_tga);
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Convert with shade table")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--shade");
    args.push_back("4");
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    bool found_png = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".png") {
        found_png = true;
        break;
      }
    }

    CHECK(found_png);
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Convert with transparency disabled")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--no-transparency");
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    bool found_png = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".png") {
        found_png = true;
        break;
      }
    }

    CHECK(found_png);
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Convert with matte hygiene enabled")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--matte");
    args.push_back("--premultiply");
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    bool found_png = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".png") {
        found_png = true;
        break;
      }
    }

    CHECK(found_png);
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Convert with premultiplied alpha")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--premultiply");
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    bool found_png = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".png") {
        found_png = true;
        break;
      }
    }

    CHECK(found_png);
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Convert with matte hygiene")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());
    args.push_back("--matte");
    args.push_back("--output");
    args.push_back(test_dir.string());

    std::string output = run_cli(args);

    bool found_png = false;
    for (const auto& entry : fs::directory_iterator(test_dir)) {
      if (entry.path().extension() == ".png") {
        found_png = true;
        break;
      }
    }

    CHECK(found_png);
    test_helpers::cleanup_test_output_dir(test_dir);
  }
}

TEST_CASE_FIXTURE(CLITestFixture, "CLI error handling")
{
  SUBCASE("Missing required arguments")
  {
    std::vector<std::string> args;
    std::string output = run_cli(args);
    // Should return non-zero exit code and show help
    CHECK(!output.empty());
  }

  SUBCASE("Non-existent input file")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back("nonexistent.art");
    args.push_back("--palette");
    args.push_back((test_dir / "PALETTE.DAT").string());

    std::string output = run_cli(args);
    // Should handle error gracefully
    CHECK(!output.empty());
    // Keep test files for manual verification - no cleanup
  }

  SUBCASE("Non-existent palette file")
  {
    auto test_dir = create_test_dir();
    std::vector<std::string> args;
    args.push_back("--input");
    args.push_back((test_dir / "TILES000.ART").string());
    args.push_back("--palette");
    args.push_back("nonexistent.dat");

    std::string output = run_cli(args);
    // Should handle error gracefully
    CHECK(!output.empty());
    // Keep test files for manual verification - no cleanup
  }
}

TEST_CASE_FIXTURE(CLITestFixture, "CLI help")
{
  SUBCASE("Show help")
  {
    std::vector<std::string> args;
    args.push_back("--help");
    std::string output = run_cli(args);
    CHECK(!output.empty());
    CHECK(output.find("Convert Build Engine ART tiles to images") !=
          std::string::npos);
    CHECK(output.find("--input") != std::string::npos);
    CHECK(output.find("--palette") != std::string::npos);
  }
}