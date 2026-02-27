// Test Harness Implementation

#include "test_harness.hpp"

#include <algorithm>
#include <cstring>

namespace art2img::test {

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

// ============================================================================
// Test Fixture
// ============================================================================

TestFixture::TestFixture() : test_name_("unknown_test") {
    init_output_dir();
}

TestFixture::TestFixture(const std::string& test_name) : test_name_(test_name) {
    init_output_dir();
}

void TestFixture::init_output_dir() {
    // Sanitize test name for filesystem
    std::string safe_name = test_name_;
    std::replace_if(
        safe_name.begin(), safe_name.end(),
        [](char c) {
            return c == ' ' || c == '/';
        },
        '_');

    output_dir_ = get_test_output_dir(safe_name);
    std::filesystem::create_directories(output_dir_);
}

std::filesystem::path TestFixture::output_dir() const {
    return output_dir_;
}

art2img::GrpFile& TestFixture::load_grp() {
    if (!grp_) {
        auto result = art2img::GrpFile::load("tests/shareware/DUKE3D.GRP");
        if (!result.ok()) {
            throw std::runtime_error("Failed to load shareware GRP: " + result.error().message);
        }
        grp_ = std::move(result.value());
    }
    return *grp_;
}

art2img::Palette& TestFixture::load_Palette() {
    if (!Palette_) {
        auto& grp = load_grp();
        auto result = art2img::Palette::from_grp(grp);
        if (!result.ok()) {
            throw std::runtime_error("Failed to load Palette: " + result.error().message);
        }
        Palette_ = std::move(result.value());
    }
    return *Palette_;
}

art2img::ArtFile& TestFixture::load_art(const std::string& name) {
    auto it = arts_.find(name);
    if (it != arts_.end()) {
        return it->second;
    }

    auto& grp = load_grp();
    auto result = art2img::ArtFile::from_grp(grp, name);
    if (!result.ok()) {
        throw std::runtime_error("Failed to load ART file '" + name +
                                 "': " + result.error().message);
    }

    auto [inserted, _] = arts_.emplace(name, std::move(result.value()));
    return inserted->second;
}

std::optional<art2img::Tile> TestFixture::find_non_empty_tile(const std::string& art_name) {
    auto& art = load_art(art_name);
    return find_non_empty_tile(art);
}

std::optional<art2img::Tile> TestFixture::find_non_empty_tile(const art2img::ArtFile& art) {
    for (size_t i = 0; i < art.tile_count(); ++i) {
        auto tile = art.get_tile(i);
        if (tile && tile->width > 0 && tile->height > 0) {
            return tile;
        }
    }
    return std::nullopt;
}

// ============================================================================
// ArtBuilder Implementation
// ============================================================================

std::vector<std::byte> ArtBuilder::build() const {
    std::vector<std::byte> data;
    data.reserve(16 + num_tiles_ * 8 + static_cast<size_t>(width_) * height_ * num_tiles_);

    // Version: 1
    data.push_back(std::byte{1});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});

    // Num tiles
    data.push_back(std::byte{num_tiles_});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});
    data.push_back(std::byte{0});

    // Tile start: 0 (or 1 if num_tiles is 0)
    uint32_t tile_start = (num_tiles_ > 0) ? 0 : 1;
    data.push_back(std::byte{static_cast<uint8_t>(tile_start & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_start >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_start >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_start >> 24) & 0xFF)});

    // Tile end: num_tiles - 1 (or 0 if num_tiles is 0)
    uint32_t tile_end = (num_tiles_ > 0) ? (num_tiles_ - 1) : 0;
    data.push_back(std::byte{static_cast<uint8_t>(tile_end & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_end >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_end >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((tile_end >> 24) & 0xFF)});

    // Widths
    for (uint8_t i = 0; i < num_tiles_; ++i) {
        data.push_back(std::byte{static_cast<uint8_t>(width_ & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((width_ >> 8) & 0xFF)});
    }

    // Heights
    for (uint8_t i = 0; i < num_tiles_; ++i) {
        data.push_back(std::byte{static_cast<uint8_t>(height_ & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((height_ >> 8) & 0xFF)});
    }

    // Picanm (4 bytes each)
    for (uint8_t i = 0; i < num_tiles_; ++i) {
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
        data.push_back(std::byte{0});
    }

    // Pixel data
    for (uint8_t t = 0; t < num_tiles_; ++t) {
        size_t pixel_count = static_cast<size_t>(width_) * height_;
        for (size_t j = 0; j < pixel_count; ++j) {
            uint8_t val = 0;
            switch (pattern_) {
            case pattern::fill:
                val = fill_value_;
                break;
            case pattern::gradient:
                val = static_cast<uint8_t>((j * 256) / (pixel_count + 1));
                break;
            case pattern::checkerboard:
                val = static_cast<uint8_t>(((j / width_) + (j % width_)) % 2 ? 255 : 0);
                break;
            }
            // Make some pixels transparent (index 255) for testing transparency
            if (j % 4 == 3) {
                val = 255;
            }
            data.push_back(std::byte{val});
        }
    }

    return data;
}

art2img::Result<art2img::ArtFile> ArtBuilder::load() const {
    auto data = build();
    return art2img::ArtFile::load(data);
}

// ============================================================================
// PaletteBuilder Implementation
// ============================================================================

PaletteBuilder& PaletteBuilder::grayscale() {
    for (size_t i = 0; i < 256; ++i) {
        uint8_t gray = static_cast<uint8_t>(i);
        data_[i * 3] = std::byte{gray};
        data_[i * 3 + 1] = std::byte{gray};
        data_[i * 3 + 2] = std::byte{gray};
    }
    return *this;
}

PaletteBuilder& PaletteBuilder::rgb_gradient() {
    for (size_t i = 0; i < 256; ++i) {
        data_[i * 3] = std::byte{static_cast<uint8_t>(i)};          // R
        data_[i * 3 + 1] = std::byte{static_cast<uint8_t>(i / 2)};  // G
        data_[i * 3 + 2] = std::byte{static_cast<uint8_t>(i / 4)};  // B
    }
    return *this;
}

PaletteBuilder& PaletteBuilder::with_color(uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
    data_[idx * 3] = std::byte{r};
    data_[idx * 3 + 1] = std::byte{g};
    data_[idx * 3 + 2] = std::byte{b};
    return *this;
}

std::vector<std::byte> PaletteBuilder::build() const {
    return data_;
}

art2img::Result<art2img::Palette> PaletteBuilder::load() const {
    auto data = build();
    return art2img::Palette::load(data);
}

// ============================================================================
// GrpBuilder Implementation
// ============================================================================

GrpBuilder& GrpBuilder::with_file(const std::string& name, std::vector<std::byte> data) {
    files_.push_back({name, std::move(data)});
    return *this;
}

GrpBuilder& GrpBuilder::with_art(const std::string& name, const ArtBuilder& builder) {
    return with_file(name, builder.build());
}

std::vector<std::byte> GrpBuilder::build() const {
    std::vector<std::byte> data;

    // Signature: "KenSilverman"
    const char* sig = "KenSilverman";
    data.insert(data.end(), reinterpret_cast<const std::byte*>(sig),
                reinterpret_cast<const std::byte*>(sig + 12));

    // File count
    uint32_t count = static_cast<uint32_t>(files_.size());
    data.push_back(std::byte{static_cast<uint8_t>(count & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((count >> 8) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((count >> 16) & 0xFF)});
    data.push_back(std::byte{static_cast<uint8_t>((count >> 24) & 0xFF)});

    // Calculate content offset (header size + directory entries)
    uint32_t content_offset = 16 + static_cast<uint32_t>(files_.size()) * 16;

    // Directory entries
    std::vector<uint32_t> offsets;
    uint32_t current_offset = content_offset;

    for (const auto& file : files_) {
        // Filename (12 bytes, null-padded)
        char name[12] = {};
        std::strncpy(name, file.name.c_str(), 11);
        name[11] = '\0';
        data.insert(data.end(), reinterpret_cast<std::byte*>(name),
                    reinterpret_cast<std::byte*>(name + 12));

        // Size (4 bytes, little-endian)
        uint32_t size = static_cast<uint32_t>(file.data.size());
        data.push_back(std::byte{static_cast<uint8_t>(size & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((size >> 8) & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((size >> 16) & 0xFF)});
        data.push_back(std::byte{static_cast<uint8_t>((size >> 24) & 0xFF)});

        offsets.push_back(current_offset);
        current_offset += size;
    }

    // File contents
    for (const auto& file : files_) {
        data.insert(data.end(), file.data.begin(), file.data.end());
    }

    return data;
}

art2img::Result<art2img::GrpFile> GrpBuilder::load() const {
    auto data = build();
    return art2img::GrpFile::load(data);
}

}  // namespace art2img::test
