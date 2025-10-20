/// @file test_io.cpp
/// @brief Unit tests for file I/O functions

#include <atomic>
#include <chrono>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <string>
#include <thread>
#include <vector>

#include <doctest/doctest.h>

#include <art2img/io.hpp>

#include "../../test_helpers.hpp"

using namespace art2img;

namespace {

/// @brief Create a temporary directory for testing
std::filesystem::path create_temp_dir() {
  // Use atomic counter to create unique directory names and avoid race conditions
  static std::atomic<int> counter{0};
  int unique_id = counter.fetch_add(1);

  std::string unique_name = "io_test_" + std::to_string(unique_id);
  auto temp_dir = test_helpers::get_unit_test_dir("io", unique_name);
  test_helpers::ensure_test_output_dir(temp_dir);
  return temp_dir;
}

/// @brief Clean up a temporary directory
void cleanup_temp_dir(const std::filesystem::path& dir) {
  test_helpers::cleanup_test_output_dir(dir);
}

/// @brief Create test binary data
std::vector<types::byte> create_test_binary_data() {
  std::vector<types::byte> data;
  for (int i = 0; i < 256; ++i) {
    data.push_back(static_cast<types::byte>(i));
  }

  // Add some pattern data
  const std::string pattern =
      "Hello, World! This is a test pattern for binary I/O.";
  for (char c : pattern) {
    data.push_back(static_cast<types::byte>(c));
  }

  return data;
}

/// @brief Create test text data
std::string create_test_text_data() {
  return "This is a test string for text file I/O.\n"
         "It contains multiple lines.\n"
         "And some special characters: áéíóú ñ 中文\n"
         "End of test data.";
}

}  // anonymous namespace

TEST_SUITE("io") {
  // ============================================================================
  // BINARY FILE I/O TESTS
  // ============================================================================

  TEST_CASE("read_binary_file - success") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "test_binary.dat";

    // Create test file with proper synchronization
    const auto test_data = create_test_binary_data();
    {
      std::ofstream file(test_file, std::ios::binary);
      file.write(reinterpret_cast<const char*>(test_data.data()),
                 test_data.size());
      file.flush();  // Explicit flush to ensure data is written
      file.close();  // Explicit close to ensure file handle is released
    }

    // Add filesystem synchronization barrier to prevent race conditions
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Test reading
    const auto result = read_binary_file(test_file);
    REQUIRE(result.has_value());

    const auto& read_data = result.value();
    CHECK(read_data.size() == test_data.size());
    CHECK(std::memcmp(read_data.data(), test_data.data(), test_data.size()) ==
          0);

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("read_binary_file - file not found") {
    const auto temp_dir = create_temp_dir();
    const auto nonexistent_file = temp_dir / "nonexistent.dat";

    const auto result = read_binary_file(nonexistent_file);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::io_failure);

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("write_binary_file - success") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "test_write.dat";

    const auto test_data = create_test_binary_data();

    // Test writing
    const auto result = write_binary_file(test_file, test_data);
    REQUIRE(result.has_value());

    // Verify file was written correctly
    std::ifstream file(test_file, std::ios::binary);
    REQUIRE(file.good());

    std::vector<types::byte> read_back(test_data.size());
    file.read(reinterpret_cast<char*>(read_back.data()), read_back.size());

    CHECK(std::memcmp(read_back.data(), test_data.data(), test_data.size()) ==
          0);

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("write_binary_file - creates directory") {
    const auto temp_dir = create_temp_dir();
    const auto nested_dir = temp_dir / "nested" / "directory";
    const auto test_file = nested_dir / "test_write.dat";

    const auto test_data = create_test_binary_data();

    // Test writing to nested directory (should create it)
    const auto result = write_binary_file(test_file, test_data);
    REQUIRE(result.has_value());

    // Verify file exists
    CHECK(std::filesystem::exists(test_file));

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("write_binary_file - permission error simulation") {
    // Test error path by trying to write to an invalid location
    // Use cross-platform approach with invalid characters
    std::filesystem::path impossible_path;

#ifdef _WIN32
    // Windows: Use path with invalid characters that are not allowed in filenames
    // These characters are universally invalid on Windows: < > : " | ? *
    impossible_path = "test_path_with_invalid_chars<>:\"|?*.dat";
#else
    // Unix-like: Use path that cannot exist (root-level impossible path)
    impossible_path = "/impossible/path/test.dat";
#endif

    const auto test_data = create_test_binary_data();

    const auto result = write_binary_file(impossible_path, test_data);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::io_failure);
  }

  // ============================================================================
  // TEXT FILE I/O TESTS
  // ============================================================================

  TEST_CASE("read_text_file - success") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "test_text.txt";

    // Create test file with proper synchronization
    const auto test_text = create_test_text_data();
    {
      std::ofstream file(test_file);
      file << test_text;
      file.flush();  // Explicit flush to ensure data is written
      file.close();  // Explicit close to ensure file handle is released
    }

    // Add filesystem synchronization barrier to prevent race conditions
    std::this_thread::sleep_for(std::chrono::milliseconds(1));

    // Test reading
    const auto result = read_text_file(test_file);
    REQUIRE(result.has_value());
    CHECK(result.value() == test_text);

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("write_text_file - success") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "test_write.txt";

    const auto test_text = create_test_text_data();

    // Test writing
    const auto result = write_text_file(test_file, test_text);
    REQUIRE(result.has_value());

    // Add filesystem synchronization barrier to prevent race conditions
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Verify file was written correctly
    std::ifstream file(test_file);
    std::string read_back;
    std::string line;
    while (std::getline(file, line)) {
      if (!read_back.empty()) {
        read_back += "\n";
      }
      read_back += line;
    }

    CHECK(read_back == test_text);

    cleanup_temp_dir(temp_dir);
  }

  // ============================================================================
  // UTILITY FUNCTION TESTS
  // ============================================================================

  TEST_CASE("check_file_readable - success") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "readable.txt";

    // Create a test file
    {
      std::ofstream file(test_file);
      file << "test";
    }

    const auto result = check_file_readable(test_file);
    REQUIRE(result.has_value());

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("check_file_readable - file not found") {
    const auto temp_dir = create_temp_dir();
    const auto nonexistent_file = temp_dir / "nonexistent.txt";

    const auto result = check_file_readable(nonexistent_file);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::io_failure);

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("check_directory_writable - success") {
    const auto temp_dir = create_temp_dir();

    const auto result = check_directory_writable(temp_dir);
    REQUIRE(result.has_value());

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("check_directory_writable - creates directory") {
    const auto temp_dir = create_temp_dir();
    const auto new_dir = temp_dir / "new_directory";

    const auto result = check_directory_writable(new_dir);
    REQUIRE(result.has_value());
    CHECK(std::filesystem::exists(new_dir));
    CHECK(std::filesystem::is_directory(new_dir));

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("ensure_directory_exists - existing directory") {
    const auto temp_dir = create_temp_dir();

    const auto result = ensure_directory_exists(temp_dir);
    REQUIRE(result.has_value());

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("ensure_directory_exists - creates new directory") {
    const auto temp_dir = create_temp_dir();
    const auto new_dir = temp_dir / "new_nested" / "directory";

    const auto result = ensure_directory_exists(new_dir);
    REQUIRE(result.has_value());
    CHECK(std::filesystem::exists(new_dir));
    CHECK(std::filesystem::is_directory(new_dir));

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("get_file_size - success") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "size_test.dat";

    const auto test_data = create_test_binary_data();
    {
      std::ofstream file(test_file, std::ios::binary);
      file.write(reinterpret_cast<const char*>(test_data.data()),
                 test_data.size());
    }

    const auto result = get_file_size(test_file);
    REQUIRE(result.has_value());
    CHECK(result.value() == test_data.size());

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("get_file_size - file not found") {
    const auto temp_dir = create_temp_dir();
    const auto nonexistent_file = temp_dir / "nonexistent.dat";

    const auto result = get_file_size(nonexistent_file);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::io_failure);

    cleanup_temp_dir(temp_dir);
  }

  // ============================================================================
  // ERROR HANDLING TESTS
  // ============================================================================

  TEST_CASE("read_binary_file - large file protection") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "large_test.dat";

    // Create a file that's larger than our protection limit
    std::ofstream file(test_file, std::ios::binary);

    // Write more than the 100MB limit (write 200MB of zeros)
    constexpr std::size_t large_size = 200 * 1024 * 1024;
    std::vector<char> zeros(1024 * 1024, 0);  // 1MB buffer

    for (int i = 0; i < 200; ++i) {
      file.write(zeros.data(), zeros.size());
    }

    file.close();

    // Try to read it - should fail due to size protection
    const auto result = read_binary_file(test_file);
    REQUIRE(!result.has_value());
    CHECK(result.error().code == errc::io_failure);

    cleanup_temp_dir(temp_dir);
  }

  // ============================================================================
  // ROUND-TRIP TESTS
  // ============================================================================

  TEST_CASE("binary file round-trip") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "roundtrip.dat";

    const auto original_data = create_test_binary_data();

    // Write data
    const auto write_result = write_binary_file(test_file, original_data);
    REQUIRE(write_result.has_value());

    // Read it back
    const auto read_result = read_binary_file(test_file);
    REQUIRE(read_result.has_value());

    const auto& read_data = read_result.value();
    CHECK(read_data.size() == original_data.size());
    CHECK(std::memcmp(read_data.data(), original_data.data(),
                      original_data.size()) == 0);

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("text file round-trip") {
    const auto temp_dir = create_temp_dir();
    const auto test_file = temp_dir / "roundtrip.txt";

    const auto original_text = create_test_text_data();

    // Write data
    const auto write_result = write_text_file(test_file, original_text);
    REQUIRE(write_result.has_value());

    // Add filesystem synchronization barrier to prevent race conditions
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Read it back
    const auto read_result = read_text_file(test_file);
    REQUIRE(read_result.has_value());

    CHECK(read_result.value() == original_text);

    cleanup_temp_dir(temp_dir);
  }

  // ============================================================================
  // EDGE CASE TESTS
  // ============================================================================

  TEST_CASE("empty file operations") {
    const auto temp_dir = create_temp_dir();
    const auto empty_file = temp_dir / "empty.dat";

    // Create empty file
    { std::ofstream file(empty_file, std::ios::binary); }

    // Read empty file
    const auto read_result = read_binary_file(empty_file);
    REQUIRE(read_result.has_value());
    CHECK(read_result.value().empty());

    // Write empty data
    std::vector<types::byte> empty_data;
    const auto write_result = write_binary_file(empty_file, empty_data);
    REQUIRE(write_result.has_value());

    cleanup_temp_dir(temp_dir);
  }

  TEST_CASE("get_filesystem_error_message") {
    // Create a test error
    std::error_code ec(errno, std::system_category());
    const std::string message = get_filesystem_error_message(ec);

    CHECK(!message.empty());
    CHECK(message.find(std::to_string(ec.value())) != std::string::npos);
  }

}  // TEST_SUITE("io")