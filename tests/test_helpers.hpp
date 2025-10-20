/// @file test_helpers.hpp
/// @brief Central test helper functions for consistent test directory
/// management

#pragma once

#include <filesystem>
#include <iostream>
#include <string>

namespace test_helpers {

/// @brief Get the main test output directory (build-relative)
/// @return Path to test output directory
inline std::filesystem::path get_test_output_dir() {
#ifdef TEST_OUTPUT_DIR
  return std::filesystem::path(TEST_OUTPUT_DIR);
#else
  // Fallback to current directory/tests_output if not defined
  return std::filesystem::current_path() / "tests_output";
#endif
}

/// @brief Get integration test output directory
/// @param test_name Name of the specific test
/// @return Path to integration test directory
inline std::filesystem::path get_integration_test_dir(
    const std::string& test_name) {
  return get_test_output_dir() / "integration" / test_name;
}

/// @brief Get unit test output directory
/// @param category Category of unit test (export, io, art, etc.)
/// @param test_name Name of the specific test
/// @return Path to unit test directory
inline std::filesystem::path get_unit_test_dir(const std::string& category,
                                               const std::string& test_name) {
  return get_test_output_dir() / "unit" / category / test_name;
}

/// @brief Get CLI test output directory
/// @param test_name Name of the CLI test
/// @return Path to CLI test directory
inline std::filesystem::path get_cli_test_dir(const std::string& test_name) {
  return get_test_output_dir() / "cli" / test_name;
}

/// @brief Ensure test output directory exists
/// @param dir Directory path to create
inline void ensure_test_output_dir(const std::filesystem::path& dir) {
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) {
    throw std::runtime_error("Failed to create test output directory: " +
                             dir.string() + " - " + ec.message());
  }
}

/// @brief Clean up test output directory
/// @param dir Directory path to remove
inline void cleanup_test_output_dir(const std::filesystem::path& dir) {
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  if (ec) {
    std::cerr << "Warning: Failed to clean up test directory: " << dir.string()
              << " - " << ec.message() << std::endl;
  }
}

/// @brief Print test output directory info (for debugging)
/// @param context Context message
inline void debug_test_output_dir(const std::string& context) {
  std::cout << "Test output directory (" << context
            << "): " << get_test_output_dir() << std::endl;
}

}  // namespace test_helpers