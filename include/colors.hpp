#pragma once

#include <cstdio>
#include <iostream>
#include <string>

#ifdef _WIN32
#  include <fcntl.h>
#  include <io.h>
#  include <windows.h>
#else
#  include <unistd.h>
#endif

namespace art2img {

class ColorOutput {
public:
  enum Color {
    RESET = 0,
    RED = 31,
    GREEN = 32,
    YELLOW = 33,
    BLUE = 34,
    MAGENTA = 35,
    CYAN = 36,
    WHITE = 37
  };

private:
  static bool use_colors_;
  static bool initialized_;

public:
  static void initialize() {
    if (initialized_)
      return;

#ifdef _WIN32
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE hErr = GetStdHandle(STD_ERROR_HANDLE);

    DWORD dwMode = 0;
    use_colors_ = (GetConsoleMode(hOut, &dwMode) != 0) && (GetConsoleMode(hErr, &dwMode) != 0);

    if (use_colors_) {
      SetConsoleMode(hOut, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
      SetConsoleMode(hErr, dwMode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }
#else
    use_colors_ = isatty(fileno(stdout)) && isatty(fileno(stderr));
#endif
    initialized_ = true;
  }

  static bool use_colors() {
    if (!initialized_)
      initialize();
    return use_colors_;
  }

  static std::string color_code(Color color) {
    if (!use_colors())
      return "";
    return "\033[" + std::to_string(color) + "m";
  }

  static std::string reset() {
    if (!use_colors())
      return "";
    return "\033[0m";
  }

  static void set_color(Color color, std::ostream& os = std::cout) {
    if (use_colors()) {
      os << color_code(color);
    }
  }

  static void reset_color(std::ostream& os = std::cout) {
    if (use_colors()) {
      os << reset();
    }
  }
};

// Static member declarations - definitions are in colors.cpp

// RAII Color helper class
class ColorGuard {
private:
  std::ostream& os_;
  bool reset_on_destroy_;

public:
  explicit ColorGuard(ColorOutput::Color color, std::ostream& os = std::cout,
                      bool reset_on_destroy = true)
      : os_(os), reset_on_destroy_(reset_on_destroy) {
    ColorOutput::set_color(color, os_);
  }

  ~ColorGuard() {
    if (reset_on_destroy_) {
      ColorOutput::reset_color(os_);
    }
  }

  // Prevent copying
  ColorGuard(const ColorGuard&) = delete;
  ColorGuard& operator=(const ColorGuard&) = delete;
};

// Convenience macros
#define COLOR_RED_STREAM(stream) art2img::ColorGuard(art2img::ColorOutput::RED, stream)
#define COLOR_GREEN_STREAM(stream) art2img::ColorGuard(art2img::ColorOutput::GREEN, stream)
#define COLOR_YELLOW_STREAM(stream) art2img::ColorGuard(art2img::ColorOutput::YELLOW, stream)
#define COLOR_BLUE_STREAM(stream) art2img::ColorGuard(art2img::ColorOutput::BLUE, stream)
#define COLOR_CYAN_STREAM(stream) art2img::ColorGuard(art2img::ColorOutput::CYAN, stream)
#define COLOR_MAGENTA_STREAM(stream) art2img::ColorGuard(art2img::ColorOutput::MAGENTA, stream)

#define COLOR_RED art2img::ColorGuard(art2img::ColorOutput::RED)
#define COLOR_GREEN art2img::ColorGuard(art2img::ColorOutput::GREEN)
#define COLOR_YELLOW art2img::ColorGuard(art2img::ColorOutput::YELLOW)
#define COLOR_BLUE art2img::ColorGuard(art2img::ColorOutput::BLUE)
#define COLOR_CYAN art2img::ColorGuard(art2img::ColorOutput::CYAN)
#define COLOR_MAGENTA art2img::ColorGuard(art2img::ColorOutput::MAGENTA)
#define COLOR_RESET art2img::ColorOutput::reset()

}  // namespace art2img