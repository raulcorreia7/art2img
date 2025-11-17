#pragma once

#include <format>
#include <iostream>
#include <sstream>
#include <string>

// For compilers that don't support std::format (like g++-12),
// we provide a simple fallback using string streams
#if __GNUC__ < 13 || (defined(__GNUC__) && __GNUC__ == 12 && __GNUC_MINOR__ < 2)
namespace std {
// Simple format fallback for g++-12 and earlier
template <typename... Args>
std::string format(const std::string& fmt, Args... args)
{
  std::ostringstream oss;
  // Basic formatting - this is a simplified version
  // For production, consider using {fmt} library or similar
  size_t pos = 0;
  size_t arg_index = 0;
  std::tuple<Args...> arg_tuple(args...);

  while (pos < fmt.length()) {
    if (pos + 1 < fmt.length() && fmt[pos] == '{' && fmt[pos + 1] == '}') {
      // Simple placeholder replacement
      if (arg_index < sizeof...(Args)) {
        oss << std::get<arg_index>(arg_tuple);
        arg_index++;
        pos += 2;
      }
      else {
        oss << "{}";
        pos++;
      }
    }
    else {
      oss << fmt[pos];
      pos++;
    }
  }
  return oss.str();
}
}  // namespace std
#endif