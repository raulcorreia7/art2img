/// @file io.cpp
/// @brief Implementation of file I/O functions for art2img
///
/// This module implements the file I/O functionality as specified in
/// Architecture §4.5 and §9. It handles:
/// - Reading and writing binary files using std::filesystem and iostreams
/// - Converting system errors to errc::io_failure
/// - Utility functions for file system operations
/// - Text file operations with UTF-8 encoding
///
/// All functions use std::expected<T, Error> for error handling with proper
/// validation according to Architecture §14 validation rules.

#include <art2img/io.hpp>
#include <fstream>
#include <sstream>
#include <system_error>

namespace art2img {

// ============================================================================
// FILE I/O FUNCTIONS
// ============================================================================

std::expected<std::vector<types::byte>, Error>
read_binary_file(const std::filesystem::path &path) {
  // Open file in binary mode
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure, "Failed to open file for reading: " + path.string());
  }

  // Get file size
  file.seekg(0, std::ios::end);
  if (!file) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure, "Failed to seek to end of file: " + path.string());
  }

  const auto file_size = file.tellg();
  if (file_size < 0) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure, "Failed to determine file size: " + path.string());
  }

  // Seek back to beginning
  file.seekg(0, std::ios::beg);
  if (!file) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure,
        "Failed to seek to beginning of file: " + path.string());
  }

  // Check file size is reasonable (prevent memory exhaustion)
  constexpr std::streamsize MAX_FILE_SIZE = 100 * 1024 * 1024; // 100MB limit
  if (file_size > MAX_FILE_SIZE) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure, "File too large: " + path.string() + " (" +
                              std::to_string(file_size) + " bytes)");
  }

  // Read file into buffer
  std::vector<types::byte> buffer(static_cast<std::size_t>(file_size));
  if (!file.read(reinterpret_cast<char *>(buffer.data()), file_size)) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure, "Failed to read file: " + path.string());
  }

  return buffer;
}

std::expected<std::monostate, Error>
write_binary_file(const std::filesystem::path &path,
                  std::span<const types::byte> data) {
  // Create parent directory if it doesn't exist
  const std::filesystem::path parent_dir = path.parent_path();
  if (!parent_dir.empty() && !std::filesystem::exists(parent_dir)) {
    std::error_code ec;
    if (!std::filesystem::create_directories(parent_dir, ec)) {
      return make_error_expected<std::monostate>(
          errc::io_failure,
          "Failed to create directory: " + parent_dir.string());
    }
  }

  // Open file in binary mode
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    return make_error_expected<std::monostate>(
        errc::io_failure, "Failed to open file for writing: " + path.string());
  }

  // Write data to file
  if (!file.write(reinterpret_cast<const char *>(data.data()),
                  static_cast<std::streamsize>(data.size()))) {
    return make_error_expected<std::monostate>(
        errc::io_failure, "Failed to write to file: " + path.string());
  }

  // Verify write was successful
  if (!file.flush()) {
    return make_error_expected<std::monostate>(
        errc::io_failure, "Failed to flush file: " + path.string());
  }

  return make_success();
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::expected<std::monostate, Error>
check_file_readable(const std::filesystem::path &path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      return make_error_expected<std::monostate>(ec, "File access error: " +
                                                         path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure, "File does not exist: " + path.string());
  }

  if (!std::filesystem::is_regular_file(path, ec)) {
    if (ec) {
      return make_error_expected<std::monostate>(ec, "File type check error: " +
                                                         path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure, "Path is not a regular file: " + path.string());
  }

  // Try to open file for reading
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    const std::error_code file_ec =
        std::error_code(errno, std::system_category());
    return make_error_expected<std::monostate>(
        file_ec, "File is not readable: " + path.string());
  }

  return make_success();
}

std::expected<std::monostate, Error>
check_directory_writable(const std::filesystem::path &path) {
  std::error_code ec;

  // Create directory if it doesn't exist
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      return make_error_expected<std::monostate>(
          ec, "Directory access error: " + path.string());
    }

    if (!std::filesystem::create_directories(path, ec)) {
      return make_error_expected<std::monostate>(
          ec, "Failed to create directory: " + path.string());
    }
  }

  if (!std::filesystem::is_directory(path, ec)) {
    if (ec) {
      return make_error_expected<std::monostate>(
          ec, "Directory type check error: " + path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure, "Path is not a directory: " + path.string());
  }

  // Try to create a test file to verify write permissions
  const std::filesystem::path test_file = path / ".art2img_write_test";
  std::ofstream file(test_file, std::ios::binary);
  if (!file) {
    const std::error_code file_ec =
        std::error_code(errno, std::system_category());
    return make_error_expected<std::monostate>(
        file_ec, "Directory is not writable: " + path.string());
  }

  file.close();

  // Clean up test file
  std::filesystem::remove(test_file, ec);
  // Ignore removal errors

  return make_success();
}

std::expected<std::monostate, Error>
ensure_directory_exists(const std::filesystem::path &path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      return make_error_expected<std::monostate>(ec, "Path access error: " +
                                                         path.string());
    }

    if (!std::filesystem::create_directories(path, ec)) {
      return make_error_expected<std::monostate>(
          ec, "Failed to create directory: " + path.string());
    }
  } else if (!std::filesystem::is_directory(path, ec)) {
    if (ec) {
      return make_error_expected<std::monostate>(ec, "Path type check error: " +
                                                         path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure,
        "Path exists but is not a directory: " + path.string());
  }

  return make_success();
}

std::expected<std::size_t, Error>
get_file_size(const std::filesystem::path &path) {
  std::error_code ec;
  const auto file_size = std::filesystem::file_size(path, ec);
  if (ec) {
    return make_error_expected<std::size_t>(
        errc::io_failure, "Failed to get file size: " + path.string());
  }

  return static_cast<std::size_t>(file_size);
}

std::expected<std::string, Error>
read_text_file(const std::filesystem::path &path) {
  // Open file in text mode
  std::ifstream file(path);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    return make_error_expected<std::string>(
        ec, "Failed to open text file for reading: " + path.string());
  }

  try {
    // Read entire file into string
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  } catch (const std::exception &e) {
    return make_error_expected<std::string>(
        errc::io_failure,
        "Failed to read text file: " + path.string() + " (" + e.what() + ")");
  }
}

std::expected<std::monostate, Error>
write_text_file(const std::filesystem::path &path, const std::string &content) {
  // Create parent directory if it doesn't exist
  const std::filesystem::path parent_dir = path.parent_path();
  if (!parent_dir.empty() && !std::filesystem::exists(parent_dir)) {
    std::error_code ec;
    if (!std::filesystem::create_directories(parent_dir, ec)) {
      return make_error_expected<std::monostate>(
          ec, "Failed to create directory: " + parent_dir.string());
    }
  }

  // Open file in text mode
  std::ofstream file(path);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    return make_error_expected<std::monostate>(
        ec, "Failed to open text file for writing: " + path.string());
  }
  // Write content to file
  file << content;
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    return make_error_expected<std::monostate>(
        ec, "Failed to write to text file: " + path.string());
  }

  // Verify write was successful
  if (!file.flush()) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    return make_error_expected<std::monostate>(
        ec, "Failed to flush text file: " + path.string());
  }

  return make_success();
}

std::string get_filesystem_error_message(const std::error_code &ec) {
  return ec.message() + " (" + std::to_string(ec.value()) + ")";
}

} // namespace art2img