#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <system_error>
#include <variant>

namespace art2img {

/// @brief Error codes specific to art2img operations
enum class errc : std::uint8_t {
  /// @brief No error (success)
  none = 0,

  /// @brief Input/output operation failed (file read/write, etc.)
  io_failure = 1,

  /// @brief ART file format is invalid or corrupted
  invalid_art = 2,

  /// @brief Palette file format is invalid or corrupted
  invalid_palette = 3,

  /// @brief Color conversion or pixel transformation failed
  conversion_failure = 4,

  /// @brief Image encoding operation failed
  encoding_failure = 5,

  /// @brief Requested operation or format is not supported
  unsupported = 6,

  /// @brief No animation data found in ART file
  no_animation = 7
};

/// @brief Forward declaration of make_error_code function
std::error_code make_error_code(errc e) noexcept;

/// @brief Error information containing code and detailed message
struct Error {
  /// @brief Platform-specific error code with art2img error category
  std::error_code code;

  /// @brief Human-readable error message with context
  std::string message;

  /// @brief Default constructor
  Error() = default;

  /// @brief Construct from error code and message
  Error(std::error_code ec, std::string msg)
      : code(std::move(ec)), message(std::move(msg)) {}

  /// @brief Construct from art2img errc and message
  Error(errc e, std::string msg)
      : code(make_error_code(e)), message(std::move(msg)) {}
};

/// @brief Custom error category for art2img error codes
class error_category : public std::error_category {
 public:
  /// @brief Get the name of this error category
  const char* name() const noexcept override;

  /// @brief Get the error message for a given error code
  std::string message(int ev) const override;

  /// @brief Get the singleton instance of the error category
  static const std::error_category& instance();
};

/// @brief Create a std::error_code from art2img errc
std::error_code make_error_code(errc e) noexcept;

/// @brief Create a failure std::expected with the given error
template <typename T = std::monostate>
std::expected<T, Error> make_error_expected(errc e,
                                            const std::string& message) {
  return std::unexpected(Error(e, message));
}

/// @brief Create a failure std::expected from a system error
template <typename T = std::monostate>
std::expected<T, Error> make_error_expected(const std::error_code& ec,
                                            const std::string& message) {
  return std::unexpected(Error(ec, message));
}

/// @brief Create a failure std::expected from an Error object
template <typename T = std::monostate>
std::expected<T, Error> make_error_expected(const Error& error) {
  return std::unexpected(error);
}

/// @brief Helper to create a success std::expected with monostate
inline std::expected<std::monostate, Error> make_success() {
  return std::monostate{};
}

/// @brief Helper to create a success std::expected with a value
template <typename T>
std::expected<T, Error> make_success(T&& value) {
  return std::forward<T>(value);
}

}  // namespace art2img

/// @brief Enable std::error_code construction from art2img::errc
namespace std {
template <>
struct is_error_code_enum<art2img::errc> : std::true_type {};
}  // namespace std