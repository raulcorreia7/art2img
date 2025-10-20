#pragma once

#include <expected>
#include <filesystem>
#include <span>
#include <vector>

#include <art2img/error.hpp>
#include <art2img/types.hpp>

namespace art2img {

// ============================================================================
// FILE I/O FUNCTIONS
// ============================================================================

/// @brief Read a binary file into a byte vector
/// @param path Path to the file to read
/// @return Expected vector of bytes on success, Error on failure
/// @note Uses std::filesystem and iostreams in binary mode
/// @note Converts system errors to errc::io_failure
std::expected<std::vector<types::byte>, Error> read_binary_file(
    const std::filesystem::path& path);

/// @brief Write binary data to a file
/// @param path Path to the file to write
/// @param data Span of bytes to write
/// @return Expected success on completion, Error on failure
/// @note Uses std::filesystem and iostreams in binary mode
/// @note Converts system errors to errc::io_failure
std::expected<std::monostate, Error> write_binary_file(
    const std::filesystem::path& path, std::span<const types::byte> data);

// ============================================================================
// UTILITY FUNCTIONS
// ============================================================================

/// @brief Check if a file exists and is readable
/// @param path Path to the file to check
/// @return Expected success on readable file, Error on failure
std::expected<std::monostate, Error> check_file_readable(
    const std::filesystem::path& path);

/// @brief Check if a directory exists and is writable
/// @param path Path to the directory to check
/// @return Expected success on writable directory, Error on failure
std::expected<std::monostate, Error> check_directory_writable(
    const std::filesystem::path& path);

/// @brief Create a directory if it doesn't exist
/// @param path Path to the directory to create
/// @return Expected success on completion, Error on failure
std::expected<std::monostate, Error> ensure_directory_exists(
    const std::filesystem::path& path);

/// @brief Get the file size in bytes
/// @param path Path to the file
/// @return Expected file size on success, Error on failure
std::expected<std::size_t, Error> get_file_size(
    const std::filesystem::path& path);

/// @brief Read a text file as a string
/// @param path Path to the text file to read
/// @return Expected string content on success, Error on failure
/// @note Uses UTF-8 encoding and handles platform-specific line endings
std::expected<std::string, Error> read_text_file(
    const std::filesystem::path& path);

/// @brief Write a string to a text file
/// @param path Path to the text file to write
/// @param content String content to write
/// @return Expected success on completion, Error on failure
/// @note Uses UTF-8 encoding and platform-specific line endings
std::expected<std::monostate, Error> write_text_file(
    const std::filesystem::path& path, const std::string& content);

/// @brief Get a human-readable error message for a filesystem error
/// @param ec The error code from a filesystem operation
/// @return Human-readable error message
std::string get_filesystem_error_message(const std::error_code& ec);

}  // namespace art2img