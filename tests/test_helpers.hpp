#pragma once

#include <cstdint>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace art2img::tests {

inline std::filesystem::path assets_root() {
#ifdef TEST_ASSET_BINARY_DIR
  static const std::filesystem::path binary_root{TEST_ASSET_BINARY_DIR};
#else
  static const std::filesystem::path binary_root{};
#endif

#ifdef TEST_ASSET_SOURCE_DIR
  static const std::filesystem::path source_root{TEST_ASSET_SOURCE_DIR};
#else
  static const std::filesystem::path source_root{"tests/assets"};
#endif

  if (!binary_root.empty() && std::filesystem::exists(binary_root)) {
    return binary_root;
  }

  if (std::filesystem::exists(source_root)) {
    return source_root;
  }

  throw std::runtime_error("Test assets directory not found");
}

inline std::filesystem::path asset_path(const std::string& filename) {
  return assets_root() / filename;
}

inline bool has_asset(const std::string& filename) {
  try {
    return std::filesystem::exists(asset_path(filename));
  } catch (...) {
    return false;
  }
}

inline std::vector<uint8_t> load_asset(const std::string& filename) {
  const auto path = asset_path(filename);
  std::ifstream file(path, std::ios::binary);
  if (!file) {
    throw std::runtime_error("Cannot open test asset: " + path.string());
  }

  file.seekg(0, std::ios::end);
  const auto size = static_cast<size_t>(file.tellg());
  file.seekg(0, std::ios::beg);

  std::vector<uint8_t> data(size);
  if (!file.read(reinterpret_cast<char*>(data.data()), static_cast<std::streamsize>(size))) {
    throw std::runtime_error("Cannot read test asset: " + path.string());
  }

  return data;
}

}  // namespace art2img::tests

inline std::vector<uint8_t> load_test_asset(const std::string& filename) {
  return art2img::tests::load_asset(filename);
}

inline std::filesystem::path test_asset_path(const std::string& filename) {
  return art2img::tests::asset_path(filename);
}

inline bool has_test_asset(const std::string& filename) {
  return art2img::tests::has_asset(filename);
}
