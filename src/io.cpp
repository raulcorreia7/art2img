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

#include <fstream>
#include <sstream>
#include <system_error>

#include <art2img/io.hpp>

#ifdef _WIN32
#include <windows.h>
#else
#include <cerrno>
#endif

namespace art2img {

// ============================================================================
// ERROR HANDLING HELPERS
// ============================================================================

/// @brief Convert system errors to appropriate art2img errors
std::error_code map_system_error(const std::error_code& ec) {
#ifdef _WIN32
  // Map specific Windows errors to art2img::io_failure
  if (ec.category() == std::system_category()) {
    switch (ec.value()) {
      case ERROR_INVALID_PARAMETER:
      case ERROR_BAD_PATHNAME:
      case ERROR_FILENAME_EXCED_RANGE:
      case ERROR_DIRECTORY:
      case ERROR_INVALID_NAME:
        return make_error_code(errc::io_failure);
      default:
        break;
    }
  }
#endif

  // Map common POSIX errors to art2img::io_failure
  if (ec.category() == std::system_category() ||
      ec.category() == std::generic_category()) {
    switch (ec.value()) {
      // File/directory access errors
      case ENOENT:   // No such file or directory
      case EACCES:   // Permission denied
      case EPERM:    // Operation not permitted
      case EROFS:    // Read-only file system
      case ELOOP:    // Too many symbolic links
      case ENOLINK:  // Link has been severed

      // File system errors
      case EIO:     // I/O error
      case ENOSPC:  // No space left on device
#ifndef _WIN32
      case EDQUOT:  // Disk quota exceeded
#endif
      case EFBIG:   // File too large
      case ENFILE:  // System file table full
      case EMFILE:  // Too many open files

      // Path and naming errors
      case ENAMETOOLONG:  // Filename too long
      case ENOTDIR:       // Not a directory
      case EISDIR:        // Is a directory
      case EINVAL:        // Invalid argument

      // Directory operation errors
      case EEXIST:     // File exists
      case ENOTEMPTY:  // Directory not empty
      case EBUSY:      // Resource busy

        // Network/mount related filesystem errors
#ifndef _WIN32
      case ESTALE:   // Stale file handle
      case EREMOTE:  // Object is remote
#ifdef EREMOTEIO
      case EREMOTEIO:  // Remote I/O error
#endif
#endif

        return make_error_code(errc::io_failure);
      default:
        break;
    }
  }

  return ec;
}

// ============================================================================
// FILE I/O FUNCTIONS
// ============================================================================

std::expected<std::vector<types::byte>, Error> read_binary_file(
    const std::filesystem::path& path) {
  // Open file in binary mode
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::vector<types::byte>>(
        mapped_ec, "Failed to open file for reading: " + path.string());
  }

  // Get file size
  file.seekg(0, std::ios::end);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::vector<types::byte>>(
        mapped_ec, "Failed to seek to end of file: " + path.string());
  }

  const auto file_size = file.tellg();
  if (file_size < 0) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::vector<types::byte>>(
        mapped_ec, "Failed to determine file size: " + path.string());
  }

  // Seek back to beginning
  file.seekg(0, std::ios::beg);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::vector<types::byte>>(
        mapped_ec, "Failed to seek to beginning of file: " + path.string());
  }

  // Check file size is reasonable (prevent memory exhaustion)
  constexpr std::streamsize MAX_FILE_SIZE = 100 * 1024 * 1024;  // 100MB limit
  if (file_size > MAX_FILE_SIZE) {
    return make_error_expected<std::vector<types::byte>>(
        errc::io_failure, "File too large: " + path.string() + " (" +
                              std::to_string(file_size) + " bytes)");
  }

  // Read file into buffer
  std::vector<types::byte> buffer(static_cast<std::size_t>(file_size));
  if (!file.read(reinterpret_cast<char*>(buffer.data()), file_size)) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::vector<types::byte>>(
        mapped_ec, "Failed to read file: " + path.string());
  }

  return buffer;
}

std::expected<std::monostate, Error> write_binary_file(
    const std::filesystem::path& path, std::span<const types::byte> data) {
  // Create parent directory if it doesn't exist
  const std::filesystem::path parent_dir = path.parent_path();
  if (!parent_dir.empty() && !std::filesystem::exists(parent_dir)) {
    std::error_code ec;
    if (!std::filesystem::create_directories(parent_dir, ec)) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Failed to create directory: " + parent_dir.string());
    }
  }

  // Open file in binary mode
  std::ofstream file(path, std::ios::binary);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Failed to open file for writing: " + path.string());
  }

  // Write data to file
  if (!file.write(reinterpret_cast<const char*>(data.data()),
                  static_cast<std::streamsize>(data.size()))) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Failed to write to file: " + path.string());
  }

  // Verify write was successful
  if (!file.flush()) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Failed to flush file: " + path.string());
  }

  return make_success();
}

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

std::expected<std::monostate, Error> check_file_readable(
    const std::filesystem::path& path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "File access error: " + path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure, "File does not exist: " + path.string());
  }

  if (!std::filesystem::is_regular_file(path, ec)) {
    if (ec) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "File type check error: " + path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure, "Path is not a regular file: " + path.string());
  }

  // Try to open file for reading
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    const std::error_code file_ec =
        std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(file_ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "File is not readable: " + path.string());
  }

  return make_success();
}

std::expected<std::monostate, Error> check_directory_writable(
    const std::filesystem::path& path) {
  std::error_code ec;

  // Create directory if it doesn't exist
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Directory access error: " + path.string());
    }

    if (!std::filesystem::create_directories(path, ec)) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Failed to create directory: " + path.string());
    }
  }

  if (!std::filesystem::is_directory(path, ec)) {
    if (ec) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Directory type check error: " + path.string());
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
    const std::error_code mapped_ec = map_system_error(file_ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Directory is not writable: " + path.string());
  }

  file.close();

  // Clean up test file
  std::filesystem::remove(test_file, ec);
  // Ignore removal errors

  return make_success();
}

std::expected<std::monostate, Error> ensure_directory_exists(
    const std::filesystem::path& path) {
  std::error_code ec;
  if (!std::filesystem::exists(path, ec)) {
    if (ec) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Path access error: " + path.string());
    }

    if (!std::filesystem::create_directories(path, ec)) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Failed to create directory: " + path.string());
    }
  } else if (!std::filesystem::is_directory(path, ec)) {
    if (ec) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Path type check error: " + path.string());
    }
    return make_error_expected<std::monostate>(
        errc::io_failure,
        "Path exists but is not a directory: " + path.string());
  }

  return make_success();
}

std::expected<std::size_t, Error> get_file_size(
    const std::filesystem::path& path) {
  std::error_code ec;
  const auto file_size = std::filesystem::file_size(path, ec);
  if (ec) {
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::size_t>(
        mapped_ec, "Failed to get file size: " + path.string());
  }

  return static_cast<std::size_t>(file_size);
}

std::expected<std::string, Error> read_text_file(
    const std::filesystem::path& path) {
  // Open file in text mode
  std::ifstream file(path);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::string>(
        mapped_ec, "Failed to open text file for reading: " + path.string());
  }

  try {
    // Read entire file into string
    std::ostringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
  } catch (const std::exception& e) {
    return make_error_expected<std::string>(
        errc::io_failure,
        "Failed to read text file: " + path.string() + " (" + e.what() + ")");
  }
}

std::expected<std::monostate, Error> write_text_file(
    const std::filesystem::path& path, const std::string& content) {
  // Create parent directory if it doesn't exist
  const std::filesystem::path parent_dir = path.parent_path();
  if (!parent_dir.empty() && !std::filesystem::exists(parent_dir)) {
    std::error_code ec;
    if (!std::filesystem::create_directories(parent_dir, ec)) {
      const std::error_code mapped_ec = map_system_error(ec);
      return make_error_expected<std::monostate>(
          mapped_ec, "Failed to create directory: " + parent_dir.string());
    }
  }

  // Open file in text mode
  std::ofstream file(path);
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Failed to open text file for writing: " + path.string());
  }
  // Write content to file
  file << content;
  if (!file) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Failed to write to text file: " + path.string());
  }

  // Verify write was successful
  if (!file.flush()) {
    const std::error_code ec = std::error_code(errno, std::system_category());
    const std::error_code mapped_ec = map_system_error(ec);
    return make_error_expected<std::monostate>(
        mapped_ec, "Failed to flush text file: " + path.string());
  }

  return make_success();
}

std::string get_filesystem_error_message(const std::error_code& ec) {
  std::string base_message =
      ec.message() + " (" + std::to_string(ec.value()) + ")";

  // Add Linux-specific context for common filesystem errors
  if (ec.category() == std::system_category() ||
      ec.category() == std::generic_category()) {
    switch (ec.value()) {
      case ENOENT:
        return base_message + " - File or directory does not exist";
      case EACCES:
        return base_message +
               " - Permission denied (check file/directory permissions)";
      case EPERM:
        return base_message +
               " - Operation not permitted (check user privileges)";
      case EROFS:
        return base_message + " - Read-only filesystem (cannot write)";
      case ENOSPC:
        return base_message + " - No space left on device";
      case EIO:
        return base_message + " - I/O error (possible hardware issue)";
      case ENAMETOOLONG:
        return base_message + " - Path or filename too long";
      case ELOOP:
        return base_message +
               " - Too many symbolic links (possible circular reference)";
      case EMFILE:
        return base_message + " - Process file descriptor limit exceeded";
      case ENFILE:
        return base_message + " - System file table full";
      default:
        return base_message;
    }
  }

  return base_message;
}

}  // namespace art2img