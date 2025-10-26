#pragma once

#include <cstdint>
#include <expected>
#include <filesystem>
#include <string>
#include <system_error>
#include <utility>
#include <variant>

namespace art2img::core {

enum class errc : std::uint8_t {
  none = 0,
  io_failure = 1,
  invalid_art = 2,
  invalid_palette = 3,
  conversion_failure = 4,
  encoding_failure = 5,
  unsupported = 6,
  no_animation = 7,
  animation_format = 8,
};

}  // namespace art2img::core

// Enable implicit conversion from errc to std::error_code
// This specialization must be visible before any use
namespace std {
template <>
struct is_error_code_enum<art2img::core::errc> : true_type {};
}  // namespace std

namespace art2img::core {

class error_category : public std::error_category {
 public:
  const char* name() const noexcept override;
  std::string message(int ev) const override;

  static const std::error_category& instance();
};

std::error_code make_error_code(errc e) noexcept;

struct Error {
  std::error_code code;
  std::string message;

  Error() = default;

  Error(std::error_code ec, std::string msg)
      : code(std::move(ec)), message(std::move(msg)) {}

  Error(errc e, std::string msg)
      : code(make_error_code(e)), message(std::move(msg)) {}
};

std::string format_error_message(const std::string& base_message,
                                 const std::string& context);

std::string format_file_error(const std::string& base_message,
                              const std::filesystem::path& file_path);

std::string format_tile_error(const std::string& base_message,
                              std::size_t tile_index);

template <typename T = std::monostate>
std::expected<T, Error> make_error_expected(errc e,
                                            const std::string& message) {
  return std::unexpected(Error(e, message));
}

template <typename T = std::monostate>
std::expected<T, Error> make_error_expected(const std::error_code& ec,
                                            const std::string& message) {
  return std::unexpected(Error(ec, message));
}

template <typename T = std::monostate>
std::expected<T, Error> make_error_expected(const Error& error) {
  return std::unexpected(error);
}

inline std::expected<std::monostate, Error> make_success() {
  return std::monostate{};
}

template <typename T>
std::expected<T, Error> make_success(T&& value) {
  return std::forward<T>(value);
}

inline Error make_error(errc code, std::string message) {
  return Error(code, std::move(message));
}

}  // namespace art2img::core
