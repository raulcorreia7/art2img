// Minimal fmtlib implementation for art2img
// Provides basic format() and print() functionality

#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <type_traits>

namespace fmt {

// Basic format function - converts arguments to string and concatenates
template<typename... Args>
std::string format(const std::string& format, Args&&... args) {
    std::ostringstream oss;
    oss << format;
    return oss.str();
}

// Basic print function
void print(const std::string& format) {
    std::cout << format;
}

// Basic print function with one argument
template<typename T>
void print(const std::string& format, const T& arg) {
    std::cout << arg;
}

// Basic print function with newline
void println(const std::string& format) {
    std::cout << format << std::endl;
}

// Basic print function with one argument and newline
template<typename T>
void println(const std::string& format, const T& arg) {
    std::cout << arg << std::endl;
}

// Basic print to stderr
void print_error(const std::string& format) {
    std::cerr << format;
}

// Basic print to stderr with one argument
template<typename T>
void print_error(const std::string& format, const T& arg) {
    std::cerr << arg;
}

// Basic print to stderr with newline
void println_error(const std::string& format) {
    std::cerr << format << std::endl;
}

// Basic print to stderr with one argument and newline
template<typename T>
void println_error(const std::string& format, const T& arg) {
    std::cerr << arg << std::endl;
}

} // namespace fmt