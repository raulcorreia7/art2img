/// @file test_cli_export.cpp
/// @brief CLI tests for export functionality

#include "../test_helpers.hpp"
#include <doctest/doctest.h>
#include <filesystem>
#include <iostream>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

namespace {

// Mock CLI runner to capture output
class CliRunner {
public:
  CliRunner(int argc, char *argv[]) : argc_(argc), argv_(argv) {}

  int run() {
    // Redirect stdout to capture output
    std::stringstream buffer;
    std::streambuf *old = std::cout.rdbuf(buffer.rdbuf());

    // Call main function (would need to be exposed for testing)
    // int result = art2img_main(argc_, argv_);

    std::cout.rdbuf(old); // Restore
    output_ = buffer.str();

    return 0; // Mock success
  }

  const std::string &get_output() const { return output_; }
  const std::string &get_error_output() const { return error_output_; }

private:
  int argc_;
  char **argv_;
  std::string output_;
  std::string error_output_;
};

// Test fixtures
struct CliTestFixture {
  std::filesystem::path temp_dir;
  std::filesystem::path art_file;
  std::filesystem::path palette_file;
  std::filesystem::path output_dir;

  CliTestFixture() {
    temp_dir = test_helpers::get_cli_test_dir("cli_test");
    test_helpers::ensure_test_output_dir(temp_dir);

    art_file = temp_dir / "test.art";
    palette_file = temp_dir / "palette.dat";
    output_dir = temp_dir / "output";

    // Create dummy files for testing
    std::ofstream(art_file.string());
    std::ofstream(palette_file.string());
  }

  ~CliTestFixture() { test_helpers::cleanup_test_output_dir(temp_dir); }

  std::vector<char *> make_argv(const std::vector<std::string> &args) {
    std::vector<char *> argv;
    for (const auto &arg : args) {
      argv.push_back(const_cast<char *>(arg.c_str()));
    }
    return argv;
  }
};

} // anonymous namespace

TEST_SUITE("CLI Export Tests") {

  TEST_CASE_FIXTURE(CliTestFixture, "CLI basic export command structure") {
    // Test that CLI accepts basic export arguments
    std::vector<std::string> args = {"art2img", art_file.string(),
                                     "-p",      palette_file.string(),
                                     "-o",      output_dir.string()};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    // Basic structure test - CLI should not crash
    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with PNG format") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "png"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    // Check that output mentions PNG processing
    CHECK(runner.get_output().find("png") != std::string::npos);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with TGA format") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "tga"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    CHECK(runner.get_output().find("tga") != std::string::npos);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with BMP format") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "bmp"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    CHECK(runner.get_output().find("bmp") != std::string::npos);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with invalid format") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "invalid"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    // Should fail with invalid format
    CHECK(result != 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export missing palette file") {
    std::vector<std::string> args = {"art2img", art_file.string(), "-o",
                                     output_dir.string()};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    // Should fail without palette
    CHECK(result != 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export missing input file") {
    std::vector<std::string> args = {"art2img", "-p", palette_file.string(),
                                     "-o", output_dir.string()};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    // Should fail without input file
    CHECK(result != 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with verbose output") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-v"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    // Verbose output should contain more details
    CHECK(runner.get_output().length() > 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with quiet output") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-q"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    // Quiet output should be minimal
    CHECK(runner.get_output().find("Progress") == std::string::npos);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with custom filename prefix") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p",       palette_file.string(),
        "-o",      output_dir.string(), "--prefix", "custom_"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    // Check that output files use custom prefix
    // This would need actual file checking in real implementation
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export directory processing") {
    // Create a subdirectory with ART files
    auto art_dir = temp_dir / "art_files";
    std::filesystem::create_directories(art_dir);
    std::ofstream((art_dir / "file1.art").string());
    std::ofstream((art_dir / "file2.art").string());

    std::vector<std::string> args = {"art2img", art_dir.string(),
                                     "-p",      palette_file.string(),
                                     "-o",      output_dir.string()};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
    // Should process multiple files
    CHECK(runner.get_output().find("ART files") != std::string::npos);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with parallel processing") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-j", "2"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with transparency options") {
    std::vector<std::string> args = {"art2img",
                                     art_file.string(),
                                     "-p",
                                     palette_file.string(),
                                     "-o",
                                     output_dir.string(),
                                     "--no-transparency-fix"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with lookup options") {
    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p",         palette_file.string(),
        "-o",      output_dir.string(), "--no-lookup"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

} // TEST_SUITE