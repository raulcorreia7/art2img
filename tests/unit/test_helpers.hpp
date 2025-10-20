/// @file test_helpers.hpp
/// @brief Central test helper functions for consistent test directory
/// management

#pragma once

#include <atomic>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <string>

namespace test_helpers {

// Global mutex for thread-safe directory operations
inline std::mutex& get_test_dir_mutex() {
  static std::mutex mutex;
  return mutex;
}

// Atomic counter for unique test identifiers
inline std::atomic<int>& get_test_counter() {
  static std::atomic<int> counter{0};
  return counter;
}

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

/// @brief Ensure test output directory exists (thread-safe)
/// @param dir Directory path to create
inline void ensure_test_output_dir(const std::filesystem::path& dir) {
  std::lock_guard<std::mutex> lock(get_test_dir_mutex());
  std::error_code ec;
  std::filesystem::create_directories(dir, ec);
  if (ec) {
    throw std::runtime_error("Failed to create test output directory: " +
                             dir.string() + " - " + ec.message());
  }
}

/// @brief Clean up test output directory (thread-safe)
/// @param dir Directory path to remove
inline void cleanup_test_output_dir(const std::filesystem::path& dir) {
  std::lock_guard<std::mutex> lock(get_test_dir_mutex());
  std::error_code ec;
  std::filesystem::remove_all(dir, ec);
  if (ec) {
    std::cerr << "Warning: Failed to clean up test directory: " << dir.string()
              << " - " << ec.message() << std::endl;
  }
}

/// @brief Generate unique test directory name (thread-safe)
/// @param prefix Optional prefix for the directory name
/// @return Unique directory name
inline std::string generate_unique_test_name(
    const std::string& prefix = "test") {
  int unique_id = get_test_counter().fetch_add(1);
  return prefix + "_" + std::to_string(unique_id);
}

/// @brief Get unique test directory path (thread-safe)
/// @param base_dir Base directory for the test
/// @param prefix Optional prefix for the unique directory name
/// @return Unique directory path
inline std::filesystem::path get_unique_test_dir(
    const std::filesystem::path& base_dir, const std::string& prefix = "test") {
  return base_dir / generate_unique_test_name(prefix);
}

/// @brief Create and ensure unique test directory (thread-safe)
/// @param base_dir Base directory for the test
/// @param prefix Optional prefix for the unique directory name
/// @return Unique directory path that has been created
inline std::filesystem::path create_unique_test_dir(
    const std::filesystem::path& base_dir, const std::string& prefix = "test") {
  auto unique_dir = get_unique_test_dir(base_dir, prefix);
  ensure_test_output_dir(unique_dir);
  return unique_dir;
}

/// @brief Print test output directory info (for debugging)
/// @param context Context message
inline void debug_test_output_dir(const std::string& context) {
  std::cout << "Test output directory (" << context
            << "): " << get_test_output_dir() << std::endl;
}

}  // namespace test_helpers