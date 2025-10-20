/// @file test_cli_export.cpp
/// @brief CLI tests for export functionality

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#else
#include <sys/wait.h>
#include <unistd.h>
#endif

#include <array>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <regex>
#include <sstream>
#include <string>
#include <vector>

#include <doctest/doctest.h>

#include "../../unit/test_helpers.hpp"

namespace {

// CLI runner that executes the actual CLI executable using secure process execution
class CliRunner {
 public:
  CliRunner(int argc, char* argv[]) : argc_(argc), argv_(argv) {}

#ifdef _WIN32
  int run() {
    // Build command line
    std::string cmd_args = "art2img_cli.exe";
    for (int i = 1; i < argc_; ++i) {
      cmd_args += " \"" + std::string(argv_[i]) + "\"";
    }

    // Security attributes for pipe
    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof(SECURITY_ATTRIBUTES);
    sa.bInheritHandle = TRUE;
    sa.lpSecurityDescriptor = NULL;

    // Create pipes for stdout and stderr
    HANDLE stdout_read = NULL, stdout_write = NULL;
    HANDLE stderr_read = NULL, stderr_write = NULL;

    if (!CreatePipe(&stdout_read, &stdout_write, &sa, 0) ||
        !CreatePipe(&stderr_read, &stderr_write, &sa, 0)) {
      error_output_ = "Failed to create pipes";
      return 1;
    }

    // Ensure the handles are not inherited by child process
    SetHandleInformation(stdout_read, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(stderr_read, HANDLE_FLAG_INHERIT, 0);

    // Process startup info
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    si.hStdError = stderr_write;
    si.hStdOutput = stdout_write;
    si.hStdInput = NULL;
    si.dwFlags |= STARTF_USESTDHANDLES;

    // Set working directory to cli subdirectory
    std::string work_dir = "cli";

    // Create process
    std::string cmd_line = cmd_args;
    if (!CreateProcessA(NULL, &cmd_line[0], NULL, NULL, TRUE, 0, NULL,
                        work_dir.c_str(), &si, &pi)) {
      CloseHandle(stdout_read);
      CloseHandle(stdout_write);
      CloseHandle(stderr_read);
      CloseHandle(stderr_write);
      error_output_ = "Failed to create process";
      return 1;
    }

    // Close write ends of pipes (child process will use them)
    CloseHandle(stdout_write);
    CloseHandle(stderr_write);

    // Read output
    char buffer[1024];
    DWORD bytes_read;

    // Read stdout
    while (
        ReadFile(stdout_read, buffer, sizeof(buffer) - 1, &bytes_read, NULL) &&
        bytes_read > 0) {
      buffer[bytes_read] = '\0';
      output_ += buffer;
    }

    // Read stderr
    while (
        ReadFile(stderr_read, buffer, sizeof(buffer) - 1, &bytes_read, NULL) &&
        bytes_read > 0) {
      buffer[bytes_read] = '\0';
      error_output_ += buffer;
    }

    // Wait for process to complete
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Get exit code
    DWORD exit_code;
    GetExitCodeProcess(pi.hProcess, &exit_code);

    // Cleanup
    CloseHandle(stdout_read);
    CloseHandle(stderr_read);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return static_cast<int>(exit_code);
  }
#else
  int run() {
    // Create pipes for capturing stdout and stderr
    int stdout_pipe[2];
    int stderr_pipe[2];

    if (pipe(stdout_pipe) == -1 || pipe(stderr_pipe) == -1) {
      error_output_ = "Failed to create pipes";
      return 1;
    }

    // Fork a new process
    pid_t pid = fork();
    if (pid == -1) {
      // Fork failed
      close(stdout_pipe[0]);
      close(stdout_pipe[1]);
      close(stderr_pipe[0]);
      close(stderr_pipe[1]);
      error_output_ = "Failed to fork process";
      return 1;
    }

    if (pid == 0) {
      // Child process
      // Close read ends of pipes
      close(stdout_pipe[0]);
      close(stderr_pipe[0]);

      // Redirect stdout and stderr to pipes
      dup2(stdout_pipe[1], STDOUT_FILENO);
      dup2(stderr_pipe[1], STDERR_FILENO);

      // Close original write ends
      close(stdout_pipe[1]);
      close(stderr_pipe[1]);

      // Build argument array for exec
      std::vector<char*> exec_args(argc_ + 1);
      exec_args[0] = const_cast<char*>("art2img_cli");
      for (int i = 1; i < argc_; ++i) {
        exec_args[i] = argv_[i];
      }
      exec_args[argc_] = nullptr;

      // Execute the CLI binary from the build directory
      execv("./cli/art2img_cli", exec_args.data());

      // If execv returns, it failed
      std::cerr << "Failed to execute CLI binary" << std::endl;
      _exit(1);
    } else {
      // Parent process
      // Close write ends of pipes
      close(stdout_pipe[1]);
      close(stderr_pipe[1]);

      // Read output from child process
      char buffer[1024];
      ssize_t bytes_read;

      // Read stdout
      while ((bytes_read = read(stdout_pipe[0], buffer, sizeof(buffer) - 1)) >
             0) {
        buffer[bytes_read] = '\0';
        output_ += buffer;
      }

      // Read stderr
      while ((bytes_read = read(stderr_pipe[0], buffer, sizeof(buffer) - 1)) >
             0) {
        buffer[bytes_read] = '\0';
        error_output_ += buffer;
      }

      // Close read ends
      close(stdout_pipe[0]);
      close(stderr_pipe[0]);

      // Wait for child process to complete
      int status;
      waitpid(pid, &status, 0);

      // Return exit code
      if (WIFEXITED(status)) {
        return WEXITSTATUS(status);
      } else {
        return 1;
      }
    }
  }
#endif

  const std::string& get_output() const { return output_; }

  const std::string& get_error_output() const { return error_output_; }

 private:
  int argc_;
  char** argv_;
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

    // Use real test assets instead of dummy files
    art_file = std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "TILES001.ART";
    palette_file = std::filesystem::path(TEST_ASSET_SOURCE_DIR) / "PALETTE.DAT";
    output_dir = temp_dir / "output";

    // Only create output directory, not the input files
    std::filesystem::create_directories(output_dir);
  }

  ~CliTestFixture() { test_helpers::cleanup_test_output_dir(temp_dir); }

  std::vector<char*> make_argv(const std::vector<std::string>& args) {
    std::vector<char*> argv;
    for (const auto& arg : args) {
      argv.push_back(const_cast<char*>(arg.c_str()));
    }
    return argv;
  }
};

}  // anonymous namespace

TEST_SUITE("CLI Export Tests") {

  TEST_CASE_FIXTURE(CliTestFixture, "CLI basic export command structure") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

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
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "png"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with TGA format") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "tga"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with BMP format") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-f", "bmp"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with invalid format") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

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
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file)) {
      return;
    }

    std::vector<std::string> args = {"art2img", art_file.string(),
                                     "-o",      output_dir.string(),
                                     "-p",      "/nonexistent/palette.dat"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    // Should fail with missing palette
    CHECK(result != 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export missing input file") {
    // Skip if test assets directory not found
    if (!std::filesystem::exists(
            std::filesystem::path(TEST_ASSET_SOURCE_DIR))) {
      return;
    }

    std::vector<std::string> args = {"art2img", "/nonexistent/file.ART",
                                     "-p",      palette_file.string(),
                                     "-o",      output_dir.string()};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    // Should fail with missing input
    CHECK(result != 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with verbose output") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-v"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with quiet output") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p", palette_file.string(),
        "-o",      output_dir.string(), "-q"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with custom filename prefix") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p",           palette_file.string(),
        "-o",      output_dir.string(), "--no-parallel"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export directory processing") {
    // Skip if test assets directory not found
    if (!std::filesystem::exists(
            std::filesystem::path(TEST_ASSET_SOURCE_DIR))) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", TEST_ASSET_SOURCE_DIR, "-p",           palette_file.string(),
        "-o",      output_dir.string(),   "--no-parallel"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with parallel processing") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {"art2img", art_file.string(),
                                     "-p",      palette_file.string(),
                                     "-o",      output_dir.string()};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

  TEST_CASE_FIXTURE(CliTestFixture, "CLI export with transparency options") {
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

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
    // Skip if test assets not found
    if (!std::filesystem::exists(art_file) ||
        !std::filesystem::exists(palette_file)) {
      return;
    }

    std::vector<std::string> args = {
        "art2img", art_file.string(),   "-p",         palette_file.string(),
        "-o",      output_dir.string(), "--no-lookup"};
    auto argv = make_argv(args);

    CliRunner runner(static_cast<int>(args.size()), argv.data());
    int result = runner.run();

    CHECK(result == 0);
  }

}  // TEST_SUITE