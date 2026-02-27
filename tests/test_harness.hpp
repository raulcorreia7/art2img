// Test Harness - Common utilities for art2img tests

#pragma once

#include <art2img.hpp>

#include "doctest.h"

#include <cstddef>
#include <filesystem>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace art2img::test {

// ============================================================================
// Shareware Detection
// ============================================================================

/// Check if shareware GRP is available for testing
bool shareware_available() noexcept;

// ============================================================================
// Test Fixture
// ============================================================================

/// Common test setup with cached resources and helper methods
class TestFixture {
   public:
    TestFixture();
    explicit TestFixture(const std::string& test_name);

    /// Get the output directory for this test
    [[nodiscard]] std::filesystem::path output_dir() const;

    /// Get the test name
    [[nodiscard]] const std::string& test_name() const {
        return test_name_;
    }

    /// Load ART file from shareware GRP (cached)
    [[nodiscard]] art2img::ArtFile& load_art(const std::string& name = "tiles000.art");

    /// Load Palette from shareware GRP (cached)
    [[nodiscard]] art2img::Palette& load_Palette();

    /// Load shareware GRP (cached)
    [[nodiscard]] art2img::GrpFile& load_grp();

    /// Find first non-empty tile in loaded ART
    /// Returns std::nullopt if no valid tile found
    [[nodiscard]] std::optional<art2img::Tile>
    find_non_empty_tile(const std::string& art_name = "tiles000.art");

    /// Find first non-empty tile in given ART file
    [[nodiscard]] static std::optional<art2img::Tile>
    find_non_empty_tile(const art2img::ArtFile& art);

   private:
    std::string test_name_;
    std::filesystem::path output_dir_;

    // Cached resources (lazy-loaded)
    std::optional<art2img::GrpFile> grp_;
    std::optional<art2img::Palette> Palette_;
    std::unordered_map<std::string, art2img::ArtFile> arts_;

    void init_output_dir();
};

// ============================================================================
// Custom Doctest Assertions
// ============================================================================

/// Assert that an ArtFile loaded successfully
#define ASSERT_ART_LOADED(result)                                         \
    do {                                                                  \
        auto&& _r = (result);                                             \
        REQUIRE(_r.ok());                                                 \
        REQUIRE_MESSAGE(_r.value().tile_count() > 0, "ART has no tiles"); \
    } while (0)

/// Assert that a GRP loaded successfully
#define ASSERT_GRP_LOADED(result)                                    \
    do {                                                             \
        auto&& _r = (result);                                        \
        REQUIRE(_r.ok());                                            \
        REQUIRE_MESSAGE(_r.value().count() > 0, "GRP has no files"); \
    } while (0)

/// Assert that a Palette loaded successfully
#define ASSERT_PALETTE_LOADED(result)                                        \
    do {                                                                     \
        auto&& _r = (result);                                                \
        REQUIRE(_r.ok());                                                    \
        uint8_t _rgba[4];                                                    \
        REQUIRE_MESSAGE(_r.value().get_color(0, _rgba), "Cannot get color"); \
    } while (0)

/// Assert that a tile is valid (non-empty dimensions)
#define ASSERT_VALID_TILE(tile_opt)                                  \
    do {                                                             \
        auto&& _t = (tile_opt);                                      \
        REQUIRE(_t.has_value());                                     \
        REQUIRE_MESSAGE(_t->width > 0, "Tile width is 0");           \
        REQUIRE_MESSAGE(_t->height > 0, "Tile height is 0");         \
        REQUIRE_MESSAGE(_t->indices != nullptr, "Tile has no data"); \
    } while (0)

/// Assert that an image was rendered successfully
#define ASSERT_IMAGE_VALID(img_result, expected_w, expected_h)              \
    do {                                                                    \
        auto&& _r = (img_result);                                           \
        REQUIRE(_r.ok());                                                   \
        REQUIRE(_r.value().width == (expected_w));                          \
        REQUIRE(_r.value().height == (expected_h));                         \
        REQUIRE_MESSAGE(!_r.value().pixels.empty(), "Image has no pixels"); \
    } while (0)

/// Assert that an encode operation succeeded
#define ASSERT_ENCODE_SUCCESS(result)                                  \
    do {                                                               \
        auto&& _r = (result);                                          \
        REQUIRE(_r.ok());                                              \
        REQUIRE_MESSAGE(!_r.value().empty(), "Encoded data is empty"); \
    } while (0)

// ============================================================================
// Test Data Builders
// ============================================================================

/// Builder for creating test ART files with fluent API
class ArtBuilder {
   public:
    ArtBuilder() = default;

    /// Set tile dimensions (default: 2x2)
    ArtBuilder& with_size(uint16_t w, uint16_t h) {
        width_ = w;
        height_ = h;
        return *this;
    }

    /// Set number of tiles (default: 1)
    ArtBuilder& with_tiles(uint8_t count) {
        num_tiles_ = count;
        return *this;
    }

    /// Set fill pattern for pixel data
    ArtBuilder& with_fill(uint8_t fill_value) {
        fill_value_ = fill_value;
        pattern_ = pattern::fill;
        return *this;
    }

    /// Use gradient pattern (default)
    ArtBuilder& with_gradient() {
        pattern_ = pattern::gradient;
        return *this;
    }

    /// Use checkerboard pattern
    ArtBuilder& with_checkerboard() {
        pattern_ = pattern::checkerboard;
        return *this;
    }

    /// Build and return ART file data
    [[nodiscard]] std::vector<std::byte> build() const;

    /// Build and directly load as ArtFile
    [[nodiscard]] art2img::Result<art2img::ArtFile> load() const;

   private:
    enum class pattern { gradient, fill, checkerboard };

    uint16_t width_ = 2;
    uint16_t height_ = 2;
    uint8_t num_tiles_ = 1;
    uint8_t fill_value_ = 0;
    pattern pattern_ = pattern::gradient;
};

/// Builder for creating test Palette data
class PaletteBuilder {
   public:
    PaletteBuilder() = default;

    /// Create grayscale Palette
    PaletteBuilder& grayscale();

    /// Create RGB gradient Palette
    PaletteBuilder& rgb_gradient();

    /// Set specific color index
    PaletteBuilder& with_color(uint8_t idx, uint8_t r, uint8_t g, uint8_t b);

    /// Build 768-byte Palette data
    [[nodiscard]] std::vector<std::byte> build() const;

    /// Build and directly load as Palette
    [[nodiscard]] art2img::Result<art2img::Palette> load() const;

   private:
    std::vector<std::byte> data_{768};
};

/// Builder for creating minimal test GRP files
class GrpBuilder {
   public:
    GrpBuilder() = default;

    /// Add a file to the GRP
    GrpBuilder& with_file(const std::string& name, std::vector<std::byte> data);

    /// Add an ART file with given builder
    GrpBuilder& with_art(const std::string& name, const ArtBuilder& builder);

    /// Build and return GRP file data
    [[nodiscard]] std::vector<std::byte> build() const;

    /// Build and directly load as GrpFile
    [[nodiscard]] art2img::Result<art2img::GrpFile> load() const;

   private:
    struct file_entry {
        std::string name;
        std::vector<std::byte> data;
    };
    std::vector<file_entry> files_;
};

// ============================================================================
// Legacy Test Utilities (from main.cpp)
// ============================================================================

// These are defined in main.cpp - declared here for use by TestFixture
void set_test_output_root(const std::string& path);
std::filesystem::path get_test_output_dir(const std::string& test_name);
std::filesystem::path get_test_output_root();
bool file_exists(const std::string& path);
size_t file_size(const std::string& path);
std::vector<std::byte> read_file_bytes(const std::string& path);
std::vector<std::byte> make_test_grp();
std::vector<std::byte> make_test_art(uint16_t width, uint16_t height, uint8_t num_tiles);
std::vector<std::byte> make_test_Palette();

}  // namespace art2img::test

// ============================================================================
// Doctest String Conversion for Types
// ============================================================================

namespace doctest {

template <>
struct StringMaker<art2img::error> {
    static String convert(const art2img::error& value) {
        return toString(static_cast<int>(value));
    }
};

}  // namespace doctest
