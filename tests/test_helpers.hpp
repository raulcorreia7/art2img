#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

inline std::vector<uint8_t> load_test_asset(const std::string& filename) {
    std::ifstream file("tests/assets/" + filename, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open test asset: " + filename);
    }

    file.seekg(0, std::ios::end);
    size_t size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> data(size);
    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        throw std::runtime_error("Cannot read test asset: " + filename);
    }

    return data;
}