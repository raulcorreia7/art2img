// Common test utilities

#pragma once

#include <art2img.hpp>

#include <cstddef>
#include <filesystem>
#include <string>
#include <vector>

namespace art2img::test {

// Test output directory management
void set_test_output_root(const std::string& path);
std::filesystem::path get_test_output_dir(const std::string& test_name);
std::filesystem::path get_test_output_root();

// File utilities
bool file_exists(const std::string& path);
size_t file_size(const std::string& path);
std::vector<std::byte> read_file_bytes(const std::string& path);

// Test data generators
std::vector<std::byte> make_test_grp();
std::vector<std::byte> make_test_art(uint16_t width = 2,
                                      uint16_t height = 2,
                                      uint8_t num_tiles = 1);
std::vector<std::byte> make_test_palette();

}  // namespace art2img::test
