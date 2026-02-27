// main.cpp
//
// Test runner and global setup for art2img tests.

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include "test_common.hpp"
#include "test_constants.hpp"

#include <art2img.hpp>

#include <cstdlib>
#include <filesystem>
#include <fstream>

namespace art2img::test {

// ============================================================================
// Test Output Directory
// ============================================================================

static std::filesystem::path g_test_output_root;

void set_test_output_root(const std::string& path) {
    g_test_output_root = path;
}

std::filesystem::path get_test_output_dir(const std::string& test_name) {
    auto path = g_test_output_root / test_name;
    std::filesystem::create_directories(path);
    return path;
}

std::filesystem::path get_test_output_root() {
    return g_test_output_root;
}

// ============================================================================
// File Utilities
// ============================================================================

bool file_exists(const std::string& path) {
    return std::filesystem::exists(path);
}

size_t file_size(const std::string& path) {
    return std::filesystem::file_size(path);
}

std::vector<std::byte> read_file_bytes(const std::string& path) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file)
        return {};
    auto size = file.tellg();
    file.seekg(0, std::ios::beg);
    std::vector<char> buffer(size);
    if (!file.read(buffer.data(), size))
        return {};
    return std::vector<std::byte>(reinterpret_cast<std::byte*>(buffer.data()),
                                  reinterpret_cast<std::byte*>(buffer.data() + size));
}

// ============================================================================
// Test Data Builders
// ============================================================================

std::vector<std::byte> make_test_grp() {
    std::vector<std::byte> data;

    // Signature
    data.insert(data.end(), reinterpret_cast<const std::byte*>(constants::GRP_SIGNATURE),
                reinterpret_cast<const std::byte*>(constants::GRP_SIGNATURE +
                                                   constants::GRP_SIGNATURE_LENGTH));

    // File count: 2
    data.push_back(std::byte{2});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});

    auto art_content = make_test_art(2, 2, 1);

    // Entry 1: test.txt
    char name1[12] = "test.txt";
    data.insert(data.end(), reinterpret_cast<std::byte*>(name1),
                reinterpret_cast<std::byte*>(name1 + 12));
    uint32_t size1 = 4;
    data.push_back(std::byte{static_cast<uint8_t>(size1 & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((size1 >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((size1 >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((size1 >> 24) & 0xFF)});

    // Entry 2: tiles.art
    char name2[12] = "tiles.art";
    data.insert(data.end(), reinterpret_cast<std::byte*>(name2),
                reinterpret_cast<std::byte*>(name2 + 12));
    uint32_t size2 = static_cast<uint32_t>(art_content.size());
    data.push_back(std::byte{static_cast<uint8_t>(size2 & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((size2 >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((size2 >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((size2 >> 24) & 0xFF)});

    // Content 1: "test"
    data.push_back(std::byte{'t'});
    data.push_back(std::byte{'e'});
    data.push_back(std::byte{'s'});
    data.push_back(std::byte{'t'});

    // Content 2: ART
    data.insert(data.end(), art_content.begin(), art_content.end());

    return data;
}

std::vector<std::byte> make_test_art(uint16_t width, uint16_t height, uint8_t num_tiles) {
    std::vector<std::byte> data;

    // Version: 1
    data.push_back(std::byte{1});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});

    // Num tiles
    data.push_back(std::byte{num_tiles});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});

    // Tile start (0 if num_tiles > 0, else 1 for empty range)
    uint32_t tile_start = (num_tiles > 0) ? 0 : 1;
    data.push_back(std::byte{static_cast<uint8_t>(tile_start & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_start >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_start >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_start >> 24) & 0xFF)});

    // Tile end
    uint32_t tile_end = (num_tiles > 0) ? (num_tiles - 1) : 0;
    data.push_back(std::byte{static_cast<uint8_t>(tile_end & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_end >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_end >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_end >> 24) & 0xFF)});

    // Widths
    for (uint8_t i = 0; i < num_tiles; ++i) {
        data.push_back(std::byte{static_cast<uint8_t>(width & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((width >> 8) & 0xFF)});
    }

    // Heights
    for (uint8_t i = 0; i < num_tiles; ++i) {
        data.push_back(std::byte{static_cast<uint8_t>(height & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((height >> 8) & 0xFF)});
    }

    // Picanm
    for (uint8_t i = 0; i < num_tiles; ++i) {
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
    }

    // Pixels: pattern 0, 1, 2, 255 (transparent)
    for (uint8_t i = 0; i < num_tiles; ++i) {
        size_t pixel_count = static_cast<size_t>(width) * height;
        for (size_t j = 0; j < pixel_count; ++j) {
            uint8_t val = (j % 4 == 3) ? 255 : static_cast<uint8_t>(j % 3);
            data.push_back(std::byte{val});
        }
    }

    return data;
}

std::vector<std::byte> make_test_palette() {
    std::vector<std::byte> data(constants::PALETTE_BASE_SIZE);
    for (size_t i = 0; i < constants::PALETTE_COLOR_COUNT; ++i) {
        data[i * 3] = std::byte{static_cast<uint8_t>(i % 64)};
        data[i * 3 + 1] = std::byte{static_cast<uint8_t>((i * 2) % 64)};
        data[i * 3 + 2] = std::byte{static_cast<uint8_t>((i * 3) % 64)};
    }
    return data;
}

// ============================================================================
// Shareware Detection
// ============================================================================

bool shareware_available() noexcept {
    static bool checked = false;
    static bool available = false;
    if (!checked) {
        std::ifstream f("tests/shareware/DUKE3D.GRP");
        available = f.good();
        checked = true;
    }
    return available;
}

}  // namespace art2img::test

// ============================================================================
// Global Setup
// ============================================================================

struct TestSetup {
    TestSetup() {
        art2img::test::g_test_output_root = "build/test_output";

        if (const char* env = std::getenv("TEST_OUTPUT_DIR")) {
            art2img::test::g_test_output_root = env;
        }

        std::filesystem::remove_all(art2img::test::g_test_output_root);
        std::filesystem::create_directories(art2img::test::g_test_output_root);
    }
} g_test_setup;
