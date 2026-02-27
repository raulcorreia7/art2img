// art2img.cpp
//
// Build Engine asset conversion library.
// Converts GRP archives and ART tile files to standard image formats.
// Used for Duke Nukem 3D, Blood, Shadow Warrior, and other Build Engine games.

#include "art2img.hpp"

#include <algorithm>
#include <cstdarg>
#include <cstring>
#include <fstream>

// STB Image Write - opt-in via ART2IMG_USE_STB_IMAGE
// Define ART2IMG_USE_STB_IMAGE to enable PNG/TGA/BMP encoding.
#ifdef ART2IMG_USE_STB_IMAGE
#ifndef STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#endif
#include "stb_image_write.h"
#endif

namespace art2img {

// ============================================================================
// Constants
// ============================================================================

// GRP format
constexpr uint32_t GRP_SIGNATURE_SIZE = 12;
constexpr const char* GRP_SIGNATURE = "KenSilverman";
constexpr uint32_t GRP_ENTRY_SIZE = 16;
constexpr uint32_t GRP_MAX_FILES = 100000;

// ART format
constexpr uint32_t ART_VERSION = 1;
constexpr uint32_t ART_HEADER_SIZE = 16;
constexpr uint32_t ART_MAX_TILES = 10000;

// Palette
constexpr size_t PALETTE_COLORS = 256;
constexpr size_t PALETTE_SIZE = 768;  // 256 colors × 3 bytes (RGB)
constexpr uint8_t TRANSPARENT_INDEX = 255;

// ============================================================================
// Forward Declarations
// ============================================================================

// Binary reading
static uint32_t read_u32le(const std::byte* p) noexcept;
static uint16_t read_u16le(const std::byte* p) noexcept;

// String utilities
static std::string format_string(const char* fmt, ...);
static std::string normalize_filename(const std::string& name);

// Validation
static Result<void> validate_grp_data(const std::vector<std::byte>& data) noexcept;
static Result<uint32_t> validate_art_data(const std::vector<std::byte>& data) noexcept;

// Rendering
static uint8_t get_pixel_index(const Tile& t, size_t x, size_t y,
                               const RenderOptions& opts) noexcept;
static Rgba lookup_color(const Palette& pal, uint8_t idx, uint8_t shade) noexcept;
static void fix_transparency(uint8_t idx, Rgba& rgba, const RenderOptions& opts) noexcept;
static void premultiply_alpha(Rgba& rgba, const RenderOptions& opts) noexcept;

// Image processing
static void apply_matte_hygiene(Image& img) noexcept;

// STB callbacks (only when STB is enabled)
#ifdef ART2IMG_USE_STB_IMAGE
struct write_ctx {
    std::vector<std::byte>* data;
    bool failed = false;
};
static void write_callback(void* ctx, void* data, int size);
#endif

// ============================================================================
// Public API: Error Handling
// ============================================================================

Error::Error(error c, std::string msg) : code(c), message(std::move(msg)) {}

bool Error::ok() const noexcept {
    return code == error::none;
}

Error::operator bool() const noexcept {
    return ok();
}

const char* to_string(error e) noexcept {
    switch (e) {
    case error::none:
        return "no error";
    case error::invalid_argument:
        return "invalid argument";
    case error::invalid_format:
        return "invalid file format";
    case error::corrupted_data:
        return "corrupted data";
    case error::io_error:
        return "io error";
    case error::not_found:
        return "not found";
    case error::out_of_memory:
        return "out of memory";
    case error::encoding_failed:
        return "encoding failed";
    case error::encoding_disabled:
        return "encoding disabled";
    }
    return "unknown error";
}

// Result<void> explicit instantiation
Result<void>::Result(Error err) : ok_(false), error_(std::move(err)) {}

bool Result<void>::ok() const noexcept {
    return ok_;
}

Result<void>::operator bool() const noexcept {
    return ok_;
}

const Error& Result<void>::error() const& {
    return error_;
}

enum error Result<void>::code() const noexcept {
    return ok_ ? error::none : error_.code;
}

const std::string& Result<void>::message() const noexcept {
    return error_.message;
}

// ============================================================================
// Public API: File I/O
// ============================================================================

Result<std::vector<std::byte>> read_file(const std::string& path) noexcept {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) {
        return Error{error::io_error, format_string("Cannot open file: '%s'", path.c_str())};
    }

    auto size = file.tellg();
    if (size < 0) {
        return Error{error::io_error, format_string("Cannot get file size: '%s'", path.c_str())};
    }

    file.seekg(0, std::ios::beg);

    std::vector<std::byte> data;
    try {
        data.resize(static_cast<size_t>(size));
    } catch (...) {
        return Error{error::out_of_memory,
                     format_string("Cannot allocate %ld bytes", static_cast<long>(size))};
    }

    if (!file.read(reinterpret_cast<char*>(data.data()), size)) {
        return Error{error::io_error, format_string("Cannot read file: '%s'", path.c_str())};
    }

    return data;
}

Result<void> write_file(const std::string& path, const std::vector<std::byte>& data) noexcept {
    return write_file(path, data.data(), data.size());
}

Result<void> write_file(const std::string& path, const std::byte* data, size_t size) noexcept {
    if (!data && size > 0) {
        return Error{error::invalid_argument, "Null data with non-zero size"};
    }

    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) {
        return Error{error::io_error, format_string("Cannot create file: '%s'", path.c_str())};
    }

    if (size > 0 && !file.write(reinterpret_cast<const char*>(data), size)) {
        return Error{error::io_error,
                     format_string("Cannot write %zu bytes to: '%s'", size, path.c_str())};
    }

    return Result<void>{};
}

// ============================================================================
// Public API: GRP Archive
// ============================================================================

class GrpFile::impl {
   public:
    std::vector<std::byte> data;
    struct entry {
        std::string name;
        size_t offset;
        size_t size;
    };
    std::vector<entry> entries;
};

GrpFile::GrpFile() noexcept = default;
GrpFile::~GrpFile() noexcept = default;
GrpFile::GrpFile(GrpFile&&) noexcept = default;
GrpFile& GrpFile::operator=(GrpFile&&) noexcept = default;

Result<GrpFile> GrpFile::load(const std::vector<std::byte>& data) noexcept {
    auto validation = validate_grp_data(data);
    if (!validation.ok()) {
        return Error{validation.code(), validation.message()};
    }

    uint32_t count = read_u32le(data.data() + GRP_SIGNATURE_SIZE);
    size_t dir_end = GRP_SIGNATURE_SIZE + 4 + static_cast<size_t>(count) * GRP_ENTRY_SIZE;

    GrpFile result;
    try {
        result.pimpl_ = std::make_unique<impl>();
        result.pimpl_->data = data;
        result.pimpl_->entries.reserve(count);
    } catch (...) {
        return Error{error::out_of_memory,
                     format_string("Cannot allocate GRP for %u files", count)};
    }

    const std::byte* dir = data.data() + GRP_SIGNATURE_SIZE + 4;
    size_t offset = dir_end;

    for (uint32_t i = 0; i < count; ++i) {
        char name_raw[13] = {};
        std::memcpy(name_raw, dir, 12);
        uint32_t size = read_u32le(dir + 12);

        if (offset + size > data.size()) {
            return Error{error::corrupted_data,
                         format_string("GRP entry %u ('%s') exceeds bounds", i, name_raw)};
        }

        try {
            result.pimpl_->entries.push_back({normalize_filename(name_raw), offset, size});
        } catch (...) {
            return Error{error::out_of_memory,
                         format_string("Cannot allocate entry '%s'", name_raw)};
        }

        offset += size;
        dir += GRP_ENTRY_SIZE;
    }

    return result;
}

Result<GrpFile> GrpFile::load(const std::string& path) noexcept {
    auto data = read_file(path);
    if (!data.ok()) {
        return Error{data.code(), format_string("%s: '%s'", data.message().c_str(), path.c_str())};
    }
    return load(data.value());
}

std::vector<std::byte> GrpFile::get(const std::string& name) const noexcept {
    if (!pimpl_)
        return {};

    std::string key = normalize_filename(name);
    for (const auto& e : pimpl_->entries) {
        if (e.name == key) {
            try {
                return std::vector<std::byte>(pimpl_->data.begin() + e.offset,
                                              pimpl_->data.begin() + e.offset + e.size);
            } catch (...) {
                return {};
            }
        }
    }
    return {};
}

bool GrpFile::has(const std::string& name) const noexcept {
    return !get(name).empty();
}

std::vector<std::string> GrpFile::list() const noexcept {
    if (!pimpl_)
        return {};

    std::vector<std::string> result;
    try {
        result.reserve(pimpl_->entries.size());
        for (const auto& e : pimpl_->entries) {
            result.push_back(e.name);
        }
    } catch (...) {}
    return result;
}

size_t GrpFile::count() const noexcept {
    return pimpl_ ? pimpl_->entries.size() : 0;
}

// ============================================================================
// Public API: ART Tiles
// ============================================================================

class ArtFile::impl {
   public:
    std::vector<std::byte> data;
    uint32_t tile_start = 0;
    std::vector<std::pair<uint16_t, uint16_t>> dims;
    std::vector<uint32_t> picanm;
    size_t pixel_offset = 0;
    size_t lookup_offset = 0;
};

ArtFile::ArtFile() noexcept = default;
ArtFile::~ArtFile() noexcept = default;
ArtFile::ArtFile(ArtFile&&) noexcept = default;
ArtFile& ArtFile::operator=(ArtFile&&) noexcept = default;

Result<ArtFile> ArtFile::load(const std::vector<std::byte>& data) noexcept {
    auto count_result = validate_art_data(data);
    if (!count_result.ok()) {
        return Error{count_result.code(), count_result.message()};
    }

    uint32_t count = count_result.value();
    uint32_t start = read_u32le(data.data() + 8);

    // Handle empty ART file
    if (count == 0) {
        ArtFile result;
        try {
            result.pimpl_ = std::make_unique<impl>();
            result.pimpl_->data = data;
            result.pimpl_->tile_start = start;
        } catch (...) {
            return Error{error::out_of_memory, "Cannot allocate empty ART"};
        }
        return result;
    }

    ArtFile result;
    try {
        result.pimpl_ = std::make_unique<impl>();
        result.pimpl_->data = data;
        result.pimpl_->tile_start = start;
        result.pimpl_->dims.reserve(count);
        result.pimpl_->picanm.reserve(count);
    } catch (...) {
        return Error{error::out_of_memory,
                     format_string("Cannot allocate ART for %u tiles", count)};
    }

    const std::byte* p = data.data() + ART_HEADER_SIZE;

    // Read widths
    for (uint32_t i = 0; i < count; ++i) {
        result.pimpl_->dims.push_back({read_u16le(p), 0});
        p += 2;
    }

    // Read heights
    for (uint32_t i = 0; i < count; ++i) {
        result.pimpl_->dims[i].second = read_u16le(p);
        p += 2;
    }

    // Read picanm
    for (uint32_t i = 0; i < count; ++i) {
        result.pimpl_->picanm.push_back(read_u32le(p));
        p += 4;
    }

    result.pimpl_->pixel_offset = p - data.data();

    // Calculate total pixels and validate
    size_t total_pixels = 0;
    for (const auto& d : result.pimpl_->dims) {
        total_pixels += static_cast<size_t>(d.first) * d.second;
    }

    if (result.pimpl_->pixel_offset + total_pixels > data.size()) {
        return Error{error::corrupted_data, "ART pixel data is truncated"};
    }

    result.pimpl_->lookup_offset = result.pimpl_->pixel_offset + total_pixels;

    return result;
}

Result<ArtFile> ArtFile::load(const std::string& path) noexcept {
    auto data = read_file(path);
    if (!data.ok()) {
        return Error{data.code(), format_string("%s: '%s'", data.message().c_str(), path.c_str())};
    }
    return load(data.value());
}

Result<ArtFile> ArtFile::from_grp(const GrpFile& grp, const std::string& name) noexcept {
    auto data = grp.get(name);
    if (data.empty()) {
        return Error{error::not_found, format_string("'%s' not found in GRP", name.c_str())};
    }
    auto result = load(data);
    if (!result.ok()) {
        return Error{result.code(),
                     format_string("%s (file: '%s')", result.message().c_str(), name.c_str())};
    }
    return result;
}

uint32_t ArtFile::tile_start() const noexcept {
    return pimpl_ ? pimpl_->tile_start : 0;
}

size_t ArtFile::tile_count() const noexcept {
    return pimpl_ ? pimpl_->dims.size() : 0;
}

std::optional<Tile> ArtFile::get_tile(size_t index) const noexcept {
    if (!pimpl_ || index >= pimpl_->dims.size()) {
        return std::nullopt;
    }

    Tile t;
    t.width = pimpl_->dims[index].first;
    t.height = pimpl_->dims[index].second;
    t.count = static_cast<size_t>(t.width) * t.height;

    // Calculate pixel offset by summing sizes of all previous tiles
    size_t offset = pimpl_->pixel_offset;
    for (size_t i = 0; i < index; ++i) {
        offset += static_cast<size_t>(pimpl_->dims[i].first) * pimpl_->dims[i].second;
    }
    t.indices = pimpl_->data.data() + offset;

    // Lookup table follows all pixel data
    if (pimpl_->data.size() > pimpl_->lookup_offset) {
        size_t lookup_off = pimpl_->lookup_offset + index * PALETTE_COLORS;
        if (lookup_off + PALETTE_COLORS <= pimpl_->data.size()) {
            t.lookup = pimpl_->data.data() + lookup_off;
            t.lookup_size = PALETTE_COLORS;
        }
    }

    return t;
}

std::optional<TileAnim> ArtFile::get_anim(size_t index) const noexcept {
    if (!pimpl_ || index >= pimpl_->picanm.size()) {
        return std::nullopt;
    }

    uint32_t raw = pimpl_->picanm[index];
    TileAnim anim;
    anim.picanm = raw;
    anim.anim_frames = raw & 0x0F;
    anim.anim_speed = (raw >> 4) & 0x0F;
    anim.anim_offset = (raw >> 8) & 0xFF;
    anim.xflip = (raw >> 16) & 1;
    anim.yflip = (raw >> 17) & 1;
    return anim;
}

std::optional<TileSize> ArtFile::get_tile_size(size_t index) const noexcept {
    if (!pimpl_ || index >= pimpl_->dims.size()) {
        return std::nullopt;
    }
    return TileSize{pimpl_->dims[index].first, pimpl_->dims[index].second};
}

// ============================================================================
// Public API: Palette
// ============================================================================

Result<Palette> Palette::load(const std::vector<std::byte>& data) noexcept {
    if (data.size() < PALETTE_SIZE) {
        return Error{error::corrupted_data, format_string("Palette too small: %zu bytes (need %zu)",
                                                          data.size(), PALETTE_SIZE)};
    }

    Palette result;
    for (size_t i = 0; i < PALETTE_COLORS; ++i) {
        uint8_t r = static_cast<uint8_t>(data[i * 3]);
        uint8_t g = static_cast<uint8_t>(data[i * 3 + 1]);
        uint8_t b = static_cast<uint8_t>(data[i * 3 + 2]);

        // Scale 6-bit (0-63) to 8-bit (0-255)
        result.colors_[i * 4 + 0] = (r << 2) | (r >> 4);
        result.colors_[i * 4 + 1] = (g << 2) | (g >> 4);
        result.colors_[i * 4 + 2] = (b << 2) | (b >> 4);
        // Index 255 is transparent
        result.colors_[i * 4 + 3] = (i == TRANSPARENT_INDEX) ? 0 : 255;
    }

    return result;
}

Result<Palette> Palette::load(const std::string& path) noexcept {
    auto data = read_file(path);
    if (!data.ok()) {
        return Error{data.code(), format_string("%s: '%s'", data.message().c_str(), path.c_str())};
    }
    return load(data.value());
}

Result<Palette> Palette::from_grp(const GrpFile& grp) noexcept {
    auto data = grp.get("palette.dat");
    if (data.empty()) {
        return Error{error::not_found, "PALETTE.DAT not found in GRP"};
    }
    if (data.size() < PALETTE_SIZE) {
        return Error{error::corrupted_data,
                     format_string("PALETTE.DAT truncated: %zu bytes", data.size())};
    }
    return load(data);
}

Result<Palette> Palette::load_extended(const std::vector<std::byte>& palette_dat,
                                       const std::vector<std::byte>& tabledata,
                                       const std::vector<std::byte>& lookup_dat) noexcept {
    auto base = load(palette_dat);
    if (!base.ok())
        return base;

    Palette result = base.value();

    // Load shade tables
    if (tabledata.size() >= PALETTE_COLORS) {
        try {
            result.shade_tables_.reserve(tabledata.size());
            for (auto b : tabledata) {
                result.shade_tables_.push_back(static_cast<uint8_t>(b));
            }
            result.num_shades_ = result.shade_tables_.size() / PALETTE_COLORS;
        } catch (...) {
            return Error{error::out_of_memory, "Cannot allocate shade tables"};
        }
    }

    // LOOKUP.DAT reserved for future use
    (void)lookup_dat;

    return result;
}

std::optional<Rgba> Palette::get_color(uint8_t idx) const noexcept {
    Rgba rgba;
    rgba.r = colors_[idx * 4 + 0];
    rgba.g = colors_[idx * 4 + 1];
    rgba.b = colors_[idx * 4 + 2];
    rgba.a = colors_[idx * 4 + 3];
    return rgba;
}

std::optional<Rgba> Palette::get_shaded_color(uint8_t color_idx, uint8_t shade_idx) const noexcept {
    if (shade_tables_.empty()) {
        return get_color(color_idx);
    }
    if (shade_idx >= num_shades_) {
        return get_color(color_idx);
    }

    uint8_t mapped = shade_tables_[shade_idx * PALETTE_COLORS + color_idx];
    return get_color(mapped);
}

size_t Palette::shade_count() const noexcept {
    return num_shades_;
}

// ============================================================================
// Public API: Rendering
// ============================================================================

Result<Image> render(const Tile& tile, const Palette& pal, const RenderOptions& opts) noexcept {
    // Validate
    if (!tile.indices) {
        return Error{error::invalid_argument, "Tile has no pixel data"};
    }
    if (tile.width == 0 || tile.height == 0) {
        return Error{error::invalid_argument,
                     format_string("Tile has zero dimensions: %ux%u", tile.width, tile.height)};
    }
    if (tile.count == 0 || tile.count > 100000000) {
        return Error{error::corrupted_data, format_string("Invalid pixel count: %zu", tile.count)};
    }

    // Allocate output
    Image result;
    result.width = tile.width;
    result.height = tile.height;
    try {
        result.pixels.resize(tile.count * 4);
    } catch (...) {
        return Error{error::out_of_memory,
                     format_string("Cannot allocate %zu bytes", tile.count * 4)};
    }

    // Convert each pixel: ART is column-major, output is row-major
    for (uint16_t y = 0; y < tile.height; ++y) {
        for (uint16_t x = 0; x < tile.width; ++x) {
            size_t dst = static_cast<size_t>(y) * tile.width + x;

            uint8_t idx = get_pixel_index(tile, x, y, opts);
            Rgba rgba = lookup_color(pal, idx, opts.shade);
            fix_transparency(idx, rgba, opts);
            premultiply_alpha(rgba, opts);

            result.pixels[dst * 4 + 0] = rgba.r;
            result.pixels[dst * 4 + 1] = rgba.g;
            result.pixels[dst * 4 + 2] = rgba.b;
            result.pixels[dst * 4 + 3] = rgba.a;
        }
    }

    if (opts.matte) {
        apply_matte_hygiene(result);
    }

    return result;
}

// ============================================================================
// Public API: Encoding
// ============================================================================

bool encoding_available() noexcept {
#ifdef ART2IMG_USE_STB_IMAGE
    return true;
#else
    return false;
#endif
}

Result<std::vector<std::byte>> encode(const Image& img, Format format) noexcept {
#ifdef ART2IMG_USE_STB_IMAGE
    if (img.pixels.empty()) {
        return Error{error::invalid_argument, "Image has no pixel data"};
    }
    if (img.width == 0 || img.height == 0) {
        return Error{error::invalid_argument,
                     format_string("Invalid dimensions: %ux%u", img.width, img.height)};
    }

    size_t expected = static_cast<size_t>(img.width) * img.height * 4;
    if (img.pixels.size() != expected) {
        return Error{error::corrupted_data, format_string("Buffer mismatch: expected %zu, got %zu",
                                                          expected, img.pixels.size())};
    }

    std::vector<std::byte> result;
    try {
        result.reserve(expected);
    } catch (...) {
        return Error{error::out_of_memory, "Cannot allocate encoding buffer"};
    }

    write_ctx ctx{&result, false};
    int ok = 0;
    const char* name = nullptr;

    switch (format) {
    case Format::png:
        ok = stbi_write_png_to_func(write_callback, &ctx, img.width, img.height, 4,
                                    img.pixels.data(), img.width * 4);
        name = "PNG";
        break;
    case Format::tga:
        ok = stbi_write_tga_to_func(write_callback, &ctx, img.width, img.height, 4,
                                    img.pixels.data());
        name = "TGA";
        break;
    case Format::bmp:
        ok = stbi_write_bmp_to_func(write_callback, &ctx, img.width, img.height, 4,
                                    img.pixels.data());
        name = "BMP";
        break;
    }

    if (!ok || ctx.failed) {
        return Error{error::encoding_failed,
                     format_string("%s encoding failed for %ux%u", name ? name : "unknown",
                                   img.width, img.height)};
    }

    return result;
#else
    (void)img;
    (void)format;
    return Error{error::encoding_disabled,
                 "Encoding disabled (define ART2IMG_USE_STB_IMAGE to enable)"};
#endif
}

// ============================================================================
// Public API: High-Level Convenience
// ============================================================================

Result<Image> extract_tile(const ArtFile& art, const Palette& pal, size_t index,
                           const RenderOptions& opts) noexcept {
    auto tile = art.get_tile(index);
    if (!tile) {
        return Error{error::invalid_argument,
                     format_string("Tile %zu out of range (count: %zu)", index, art.tile_count())};
    }
    return render(*tile, pal, opts);
}

Result<std::vector<Image>> extract_all(const ArtFile& art, const Palette& pal,
                                       const RenderOptions& opts) noexcept {
    size_t n = art.tile_count();
    if (n == 0) {
        return Error{error::invalid_argument, "ART has no tiles"};
    }

    std::vector<Image> result;
    try {
        result.reserve(n);
    } catch (...) {
        return Error{error::out_of_memory, format_string("Cannot allocate %zu images", n)};
    }

    for (size_t i = 0; i < n; ++i) {
        auto img = extract_tile(art, pal, i, opts);
        if (!img.ok()) {
            return Error{img.code(), format_string("Tile %zu: %s", i, img.message().c_str())};
        }
        try {
            result.push_back(std::move(img.value()));
        } catch (...) {
            return Error{error::out_of_memory, format_string("Cannot store tile %zu", i)};
        }
    }

    return result;
}

Result<std::vector<Image>> extract_range(const ArtFile& art, const Palette& pal, size_t start,
                                         size_t end, const RenderOptions& opts) noexcept {
    size_t count = art.tile_count();

    if (start >= count) {
        return Error{error::invalid_argument,
                     format_string("Start %zu >= count %zu", start, count)};
    }
    if (end > count)
        end = count;
    if (start >= end) {
        return Error{error::invalid_argument,
                     format_string("Invalid range: [%zu, %zu)", start, end)};
    }

    std::vector<Image> result;
    try {
        result.reserve(end - start);
    } catch (...) {
        return Error{error::out_of_memory, "Cannot allocate result"};
    }

    for (size_t i = start; i < end; ++i) {
        auto img = extract_tile(art, pal, i, opts);
        if (!img.ok()) {
            return Error{img.code(), img.message()};
        }
        try {
            result.push_back(std::move(img.value()));
        } catch (...) {
            return Error{error::out_of_memory, format_string("Cannot store tile %zu", i)};
        }
    }

    return result;
}

Result<size_t> convert_art(const ArtFile& art, const Palette& pal,
                           const ConvertOptions& opts) noexcept {
    size_t count = art.tile_count();
    if (count == 0) {
        return Error{error::invalid_argument, "ART has no tiles"};
    }

    size_t written = 0;

    for (size_t i = 0; i < count; ++i) {
        auto size = art.get_tile_size(i);
        if (!size || size->width == 0 || size->height == 0) {
            continue;
        }

        auto img = extract_tile(art, pal, i, opts.render);
        if (!img.ok()) {
            return Error{img.code(), format_string("Tile %zu: %s", i, img.message().c_str())};
        }

        auto data = encode(img.value(), opts.format);
        if (!data.ok()) {
            return Error{data.code(), format_string("Tile %zu: %s", i, data.message().c_str())};
        }

        std::string filename = opts.output_prefix + std::to_string(i) + opts.output_suffix;
        auto write = write_file(filename, data.value());
        if (!write.ok()) {
            return Error{write.code(), format_string("Tile %zu: %s", i, write.message().c_str())};
        }

        ++written;
    }

    return written;
}

// ============================================================================
// Internal: Binary Reading
// ============================================================================

static uint32_t read_u32le(const std::byte* p) noexcept {
    return static_cast<uint32_t>(p[0]) | (static_cast<uint32_t>(p[1]) << 8) |
           (static_cast<uint32_t>(p[2]) << 16) | (static_cast<uint32_t>(p[3]) << 24);
}

static uint16_t read_u16le(const std::byte* p) noexcept {
    return static_cast<uint16_t>(p[0]) | (static_cast<uint16_t>(p[1]) << 8);
}

// ============================================================================
// Internal: String Utilities
// ============================================================================

static std::string format_string(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list copy;
    va_copy(copy, args);
    int size = std::vsnprintf(nullptr, 0, fmt, copy);
    va_end(copy);

    if (size < 0)
        return "[format error]";

    std::string result(size, '\0');
    std::vsnprintf(&result[0], size + 1, fmt, args);
    va_end(args);

    return result;
}

static std::string normalize_filename(const std::string& name) {
    std::string result = name;
    while (!result.empty() && (result.back() == '\0' || result.back() == ' ')) {
        result.pop_back();
    }
    std::transform(result.begin(), result.end(), result.begin(), [](unsigned char c) {
        return static_cast<char>(std::tolower(c));
    });
    return result;
}

// ============================================================================
// Internal: Validation
// ============================================================================

static Result<void> validate_grp_data(const std::vector<std::byte>& data) noexcept {
    if (data.empty()) {
        return Error{error::invalid_argument, "GRP data is empty"};
    }
    if (data.size() < GRP_SIGNATURE_SIZE + 4) {
        return Error{error::corrupted_data, format_string("GRP too small: %zu bytes", data.size())};
    }
    if (std::memcmp(data.data(), GRP_SIGNATURE, GRP_SIGNATURE_SIZE) != 0) {
        char sig[13] = {};
        std::memcpy(sig, data.data(), GRP_SIGNATURE_SIZE);
        return Error{error::invalid_format,
                     format_string("Bad signature: expected '%s', got '%s'", GRP_SIGNATURE, sig)};
    }

    uint32_t count = read_u32le(data.data() + GRP_SIGNATURE_SIZE);
    if (count > GRP_MAX_FILES) {
        return Error{error::corrupted_data,
                     format_string("Too many files: %u (max %u)", count, GRP_MAX_FILES)};
    }

    size_t dir_size = static_cast<size_t>(count) * GRP_ENTRY_SIZE;
    if (data.size() < GRP_SIGNATURE_SIZE + 4 + dir_size) {
        return Error{error::corrupted_data,
                     format_string("Directory truncated: need %zu, have %zu",
                                   GRP_SIGNATURE_SIZE + 4 + dir_size, data.size())};
    }

    return Result<void>{};
}

static Result<uint32_t> validate_art_data(const std::vector<std::byte>& data) noexcept {
    if (data.size() < ART_HEADER_SIZE) {
        return Error{error::corrupted_data, format_string("ART too small: %zu bytes", data.size())};
    }

    uint32_t version = read_u32le(data.data());
    if (version != ART_VERSION) {
        return Error{error::invalid_format,
                     format_string("Unsupported version: %u (need %u)", version, ART_VERSION)};
    }

    uint32_t num_field = read_u32le(data.data() + 4);
    uint32_t start = read_u32le(data.data() + 8);
    uint32_t end = read_u32le(data.data() + 12);

    // Zero num_field means empty ART
    if (num_field == 0) {
        return static_cast<uint32_t>(0);
    }

    if (end < start) {
        return Error{error::corrupted_data,
                     format_string("Bad range: end %u < start %u", end, start)};
    }

    uint32_t count = end - start + 1;
    if (count > ART_MAX_TILES) {
        return Error{error::corrupted_data,
                     format_string("Too many tiles: %u (max %u)", count, ART_MAX_TILES)};
    }

    size_t header_size = ART_HEADER_SIZE + static_cast<size_t>(count) * 8;
    if (data.size() < header_size) {
        return Error{error::corrupted_data, format_string("Header truncated: need %zu, have %zu",
                                                          header_size, data.size())};
    }

    return count;
}

// ============================================================================
// Internal: Rendering Helpers
// ============================================================================

static uint8_t get_pixel_index(const Tile& t, size_t x, size_t y,
                               const RenderOptions& opts) noexcept {
    // ART stores pixels column-major
    size_t src = x * t.height + y;
    uint8_t idx = static_cast<uint8_t>(t.indices[src]);

    // Apply per-tile lookup table if present
    if (opts.apply_lookup && t.lookup && src < t.lookup_size) {
        idx = static_cast<uint8_t>(t.lookup[idx]);
    }

    return idx;
}

static Rgba lookup_color(const Palette& pal, uint8_t idx, uint8_t shade) noexcept {
    if (shade > 0 && pal.shade_count() > 0) {
        uint8_t shade_idx = std::min(shade, static_cast<uint8_t>(pal.shade_count() - 1));
        return pal.get_shaded_color(idx, shade_idx).value_or(Rgba{});
    }
    return pal.get_color(idx).value_or(Rgba{});
}

static void fix_transparency(uint8_t idx, Rgba& rgba, const RenderOptions& opts) noexcept {
    if (idx == TRANSPARENT_INDEX) {
        rgba.a = opts.fix_transparency ? 0 : 255;
    }
}

static void premultiply_alpha(Rgba& rgba, const RenderOptions& opts) noexcept {
    if (opts.premultiply && rgba.a < 255) {
        rgba.r = (rgba.r * rgba.a) / 255;
        rgba.g = (rgba.g * rgba.a) / 255;
        rgba.b = (rgba.b * rgba.a) / 255;
    }
}

// ============================================================================
// Internal: Image Processing
// ============================================================================

static void apply_matte_hygiene(Image& img) noexcept {
    size_t w = img.width;
    size_t h = img.height;
    if (w < 3 || h < 3)
        return;

    // Extract alpha channel
    std::vector<uint8_t> alpha(w * h);
    for (size_t i = 0; i < w * h; ++i) {
        alpha[i] = img.pixels[i * 4 + 3];
    }

    // Erode: shrink edges by taking minimum of neighbors
    std::vector<uint8_t> eroded = alpha;
    for (size_t y = 1; y < h - 1; ++y) {
        for (size_t x = 1; x < w - 1; ++x) {
            size_t i = y * w + x;
            if (alpha[i] == 0)
                continue;
            uint8_t min_neighbor =
                std::min({alpha[i - w], alpha[i + w], alpha[i - 1], alpha[i + 1]});
            eroded[i] = min_neighbor;
        }
    }

    // Blur: smooth by averaging 3x3 neighborhood
    for (size_t y = 1; y < h - 1; ++y) {
        for (size_t x = 1; x < w - 1; ++x) {
            size_t i = y * w + x;
            uint32_t sum = 0;
            for (int dy = -1; dy <= 1; ++dy) {
                for (int dx = -1; dx <= 1; ++dx) {
                    sum += eroded[(y + dy) * w + (x + dx)];
                }
            }
            img.pixels[i * 4 + 3] = static_cast<uint8_t>(sum / 9);
        }
    }
}

// ============================================================================
// Internal: STB Callbacks
// ============================================================================

#ifdef ART2IMG_USE_STB_IMAGE
static void write_callback(void* ctx, void* data, int size) {
    auto* c = static_cast<write_ctx*>(ctx);
    auto* bytes = static_cast<const std::byte*>(data);
    try {
        c->data->insert(c->data->end(), bytes, bytes + size);
    } catch (...) {
        c->failed = true;
    }
}
#endif

}  // namespace art2img
