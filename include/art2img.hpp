// art2img.hpp
//
// Build Engine asset conversion library.
//
// Converts GRP archives and ART tile files to standard image formats.
// Used for Duke Nukem 3D, Blood, Shadow Warrior, and other Build Engine games.
//
// Example usage:
//   auto grp = GrpFile::load("duke3d.grp").value();
//   auto pal = Palette::from_grp(grp).value();
//   auto art = ArtFile::from_grp(grp, "tiles000.art").value();
//   auto img = extract_tile(art, pal, 0).value();

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <variant>
#include <vector>

namespace art2img {

// ============================================================================
// Error Handling
// ============================================================================

enum class error {
    none = 0,
    invalid_argument,
    invalid_format,
    corrupted_data,
    io_error,
    not_found,
    out_of_memory,
    encoding_failed,
    encoding_disabled
};

const char* to_string(error e) noexcept;

struct Error {
    error code = error::none;
    std::string message;

    Error() = default;
    Error(error c, std::string msg);
    bool ok() const noexcept;
    explicit operator bool() const noexcept;
};

// ============================================================================
// Result Type
// ============================================================================

template <typename T>
class Result {
    std::variant<T, Error> data_;

   public:
    Result(T val) : data_(std::move(val)) {}
    Result(Error err) : data_(std::move(err)) {}

    bool ok() const noexcept {
        return std::holds_alternative<T>(data_);
    }
    explicit operator bool() const noexcept {
        return ok();
    }

    T& value() & {
        return std::get<T>(data_);
    }
    const T& value() const& {
        return std::get<T>(data_);
    }
    T&& value() && {
        return std::get<T>(std::move(data_));
    }

    const Error& error() const& {
        return std::get<Error>(data_);
    }
    Error&& error() && {
        return std::get<Error>(std::move(data_));
    }

    enum error code() const noexcept {
        return ok() ? error::none : std::get<Error>(data_).code;
    }
    const std::string& message() const noexcept {
        static const std::string empty;
        return ok() ? empty : std::get<Error>(data_).message;
    }
};

template <>
class Result<void> {
    bool ok_ = true;
    Error error_;

   public:
    Result() = default;
    Result(Error err);

    bool ok() const noexcept;
    explicit operator bool() const noexcept;

    const Error& error() const&;
    enum error code() const noexcept;
    const std::string& message() const noexcept;
};

// ============================================================================
// Core Types
// ============================================================================

struct Rgba {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
};

struct TileSize {
    uint16_t width = 0;
    uint16_t height = 0;
};

// ============================================================================
// GRP Archive
// ============================================================================

class GrpFile {
   public:
    static Result<GrpFile> load(const std::vector<std::byte>& data) noexcept;
    static Result<GrpFile> load(const std::string& path) noexcept;

    GrpFile() noexcept;
    ~GrpFile() noexcept;
    GrpFile(GrpFile&& other) noexcept;
    GrpFile& operator=(GrpFile&& other) noexcept;
    GrpFile(const GrpFile&) = delete;
    GrpFile& operator=(const GrpFile&) = delete;

    std::vector<std::byte> get(const std::string& name) const noexcept;
    bool has(const std::string& name) const noexcept;
    std::vector<std::string> list() const noexcept;
    size_t count() const noexcept;

   private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

// ============================================================================
// ART Tiles
// ============================================================================

struct Tile {
    uint16_t width = 0;
    uint16_t height = 0;
    const std::byte* indices = nullptr;
    size_t count = 0;
    const std::byte* lookup = nullptr;
    size_t lookup_size = 0;
};

struct TileAnim {
    uint32_t picanm = 0;
    int anim_frames = 0;
    int anim_speed = 0;
    int anim_offset = 0;
    bool xflip = false;
    bool yflip = false;
};

class ArtFile {
   public:
    static Result<ArtFile> load(const std::vector<std::byte>& data) noexcept;
    static Result<ArtFile> load(const std::string& path) noexcept;
    static Result<ArtFile> from_grp(const GrpFile& grp, const std::string& name) noexcept;

    ArtFile() noexcept;
    ~ArtFile() noexcept;
    ArtFile(ArtFile&& other) noexcept;
    ArtFile& operator=(ArtFile&& other) noexcept;
    ArtFile(const ArtFile&) = delete;
    ArtFile& operator=(const ArtFile&) = delete;

    uint32_t tile_start() const noexcept;
    size_t tile_count() const noexcept;
    std::optional<Tile> get_tile(size_t index) const noexcept;
    std::optional<TileAnim> get_anim(size_t index) const noexcept;
    std::optional<TileSize> get_tile_size(size_t index) const noexcept;

   private:
    class impl;
    std::unique_ptr<impl> pimpl_;
};

// ============================================================================
// Palette
// ============================================================================

struct Image;
struct RenderOptions;

class Palette {
   public:
    static Result<Palette> load(const std::vector<std::byte>& data) noexcept;
    static Result<Palette> load(const std::string& path) noexcept;
    static Result<Palette> from_grp(const GrpFile& grp) noexcept;
    static Result<Palette> load_extended(const std::vector<std::byte>& palette_dat,
                                         const std::vector<std::byte>& tabledata,
                                         const std::vector<std::byte>& lookup_dat = {}) noexcept;

    std::optional<Rgba> get_color(uint8_t idx) const noexcept;
    std::optional<Rgba> get_shaded_color(uint8_t color_idx, uint8_t shade_idx) const noexcept;
    size_t shade_count() const noexcept;

   private:
    friend Result<Image> render(const Tile&, const Palette&, const RenderOptions&) noexcept;

    std::array<uint8_t, 1024> colors_ = {};
    std::vector<uint8_t> shade_tables_;
    size_t num_shades_ = 0;
};

// ============================================================================
// Rendering
// ============================================================================

struct RenderOptions {
    bool apply_lookup = false;
    uint8_t shade = 0;
    bool fix_transparency = true;
    bool premultiply = false;
    bool matte = false;
};

struct Image {
    uint16_t width = 0;
    uint16_t height = 0;
    std::vector<uint8_t> pixels;
};

Result<Image> render(const Tile& tile, const Palette& palette,
                     const RenderOptions& options = {}) noexcept;

// ============================================================================
// Encoding
// ============================================================================

enum class Format { png, tga, bmp };

// Returns true if encoding is available (ART2IMG_USE_STB_IMAGE defined).
bool encoding_available() noexcept;

// Encodes an image to bytes. Returns error if encoding is disabled.
Result<std::vector<std::byte>> encode(const Image& image, Format format) noexcept;

// ============================================================================
// File I/O
// ============================================================================

Result<std::vector<std::byte>> read_file(const std::string& path) noexcept;
Result<void> write_file(const std::string& path, const std::vector<std::byte>& data) noexcept;
Result<void> write_file(const std::string& path, const std::byte* data, size_t size) noexcept;

// ============================================================================
// High-Level API
// ============================================================================

struct ConvertOptions {
    Format format = Format::png;
    std::string output_prefix = "tile_";
    std::string output_suffix = ".png";
    RenderOptions render = {};
    bool verbose = false;
};

Result<Image> extract_tile(const ArtFile& art, const Palette& palette, size_t index,
                           const RenderOptions& options = {}) noexcept;

Result<std::vector<Image>> extract_all(const ArtFile& art, const Palette& palette,
                                       const RenderOptions& options = {}) noexcept;

Result<std::vector<Image>> extract_range(const ArtFile& art, const Palette& palette, size_t start,
                                         size_t end, const RenderOptions& options = {}) noexcept;

Result<size_t> convert_art(const ArtFile& art, const Palette& palette,
                           const ConvertOptions& options) noexcept;

}  // namespace art2img
