#pragma once

// This header provides legacy API compatibility for art2img v2.0+
// It recreates the original API surface while forwarding to the new vNext modules

#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <cstdint>

// Include the new vNext modules that we'll forward to
#include <art2img/types.hpp>
#include <art2img/art.hpp>
#include <art2img/palette.hpp>
#include <art2img/convert.hpp>
#include <art2img/encode.hpp>
#include <art2img/io.hpp>
#include <art2img/error.hpp>

namespace art2img {
namespace legacy {

// ============================================================================
// LEGACY EXCEPTIONS
// ============================================================================

/// @brief Legacy exception class for backward compatibility
class ArtException : public std::runtime_error {
public:
    explicit ArtException(const std::string& message) : std::runtime_error(message) {}
};

// ============================================================================
// LEGACY IMAGE FORMAT ENUM
// ============================================================================

/// @brief Legacy image format enumeration (matches original API)
enum class ImageFormat { PNG, TGA, BMP };

// ============================================================================
// LEGACY STRUCTS
// ============================================================================

/// @brief Legacy extraction result structure
struct ExtractionResult {
    bool success = false;
    std::string error_message;
    std::vector<uint8_t> image_data;
    std::string format;  // "png", "tga", or "bmp"
    uint32_t tile_index = 0;
    uint16_t width = 0;
    uint16_t height = 0;

    // Animation data
    uint32_t anim_frames = 0;
    uint32_t anim_type = 0;
    int8_t x_offset = 0;
    int8_t y_offset = 0;
    uint32_t anim_speed = 0;
    uint32_t other_flags = 0;
};

// ============================================================================
// LEGACY ART FILE STRUCTURES
// ============================================================================

/// @brief Legacy ArtFile class (wrapper around new ArtData)
class ArtFile {
public:
    /// @brief Legacy Header structure (matches original)
    struct Header {
        uint32_t version;
        uint32_t start_tile;
        uint32_t end_tile;
        uint32_t num_tiles;

        constexpr bool is_valid() const {
            return version == 1 && end_tile >= start_tile;
        }
    };

    /// @brief Legacy Tile structure (matches original)
    struct Tile {
        uint16_t width;
        uint16_t height;
        uint32_t anim_data;
        uint32_t offset;

        constexpr uint32_t size() const {
            return static_cast<uint32_t>(width) * height;
        }
        constexpr bool is_empty() const {
            return size() == 0;
        }

        // Animation data accessors
        constexpr uint32_t anim_frames() const {
            return anim_data & 0x3F;
        }
        constexpr uint32_t anim_type() const {
            return (anim_data >> 6) & 0x03;
        }
        constexpr int8_t x_offset() const {
            return static_cast<int8_t>((anim_data >> 8) & 0xFF);
        }
        constexpr int8_t y_offset() const {
            return static_cast<int8_t>((anim_data >> 16) & 0xFF);
        }
        constexpr uint32_t anim_speed() const {
            return (anim_data >> 24) & 0x0F;
        }
        constexpr uint32_t other_flags() const {
            return anim_data >> 28;
        }
    };

    // Constructors
    ArtFile();
    explicit ArtFile(const std::filesystem::path& filename);
    explicit ArtFile(const uint8_t* data, size_t size);

    // Non-copyable, movable
    ArtFile(const ArtFile&) = delete;
    ArtFile& operator=(const ArtFile&) = delete;
    ArtFile(ArtFile&&) = default;
    ArtFile& operator=(ArtFile&&) = default;

    // Unified loading operations
    bool load(const std::filesystem::path& filename);
    bool load(const uint8_t* data, size_t size);
    void close();

    bool read_header();
    bool read_tile_metadata();
    bool read_tile_data(uint32_t index, std::vector<uint8_t>& buffer);

    // Accessors
    const Header& header() const;
    const std::vector<Tile>& tiles() const;
    bool is_open() const;
    const std::filesystem::path& filename() const;

    // Memory data access for zero-copy operations
    const uint8_t* data() const;
    size_t data_size() const;
    bool has_data() const;

private:
    // Internal data storage
    std::unique_ptr<ArtData> art_data_;
    Header legacy_header_;
    std::vector<Tile> legacy_tiles_;
    std::filesystem::path filename_;
    bool is_open_ = false;

    // Helper methods to convert between new and old formats
    void convert_art_data_to_legacy();
    Tile convert_tile_view_to_legacy(const TileView& view) const;
};

// ============================================================================
// LEGACY PALETTE CLASS
// ============================================================================

/// @brief Legacy Palette class (wrapper around new Palette)
class Palette {
public:
    static constexpr size_t SIZE = 256 * 3;  // 256 colors, 3 components each

    Palette();

    // Non-copyable, movable
    Palette(const Palette&) = delete;
    Palette& operator=(const Palette&) = delete;
    Palette(Palette&&) = default;
    Palette& operator=(Palette&&) = default;

    // File-based operations
    bool load_from_file(const std::filesystem::path& filename);
    void load_build_engine_default();
    void load_blood_default();
    void load_duke3d_default();

    // Memory-based operations
    bool load_from_memory(const uint8_t* data, size_t size);

    // Get palette data in BGR format (for TGA/PNG output)
    std::vector<uint8_t> get_bgr_data() const;

    // Accessors
    const std::vector<uint8_t>& raw_data() const;
    bool is_loaded() const;

    // Get specific color component
    uint8_t get_red(size_t index) const;
    uint8_t get_green(size_t index) const;
    uint8_t get_blue(size_t index) const;

private:
    std::unique_ptr<::art2img::Palette> palette_;
    std::vector<uint8_t> raw_data_;
    bool loaded_ = false;

    // Helper methods
    void update_raw_data();
};

// ============================================================================
// LEGACY IMAGE WRITER
// ============================================================================

/// @brief Legacy ImageWriter class (wrapper around new encoding functions)
class ImageWriter {
public:
    struct Options {
        bool enable_alpha = true;        // Enable alpha channel support (PNG only)
        bool premultiply_alpha = false;  // Apply premultiplication for upscaling (PNG only)
        bool matte_hygiene = false;      // Apply alpha matte hygiene (erode + blur) (PNG only)
        bool fix_transparency = true;    // Enable magenta transparency processing (PNG only)

        Options() {}
    };

    // Write image to file
    static bool write_image(const std::filesystem::path& filename, ImageFormat format,
                            const Palette& palette, const ArtFile::Tile& tile,
                            const uint8_t* pixel_data, size_t pixel_data_size,
                            const Options& options = Options());

    // Write image to memory
    static bool write_image_to_memory(std::vector<uint8_t>& output, ImageFormat format,
                                      const Palette& palette, const ArtFile::Tile& tile,
                                      const uint8_t* pixel_data, size_t pixel_data_size,
                                      const Options& options = Options());

    // Public for testing
    static constexpr bool is_magenta(uint8_t r, uint8_t g, uint8_t b) {
        // Magenta detection: r8≥250, b8≥250, g8≤5
        return (r >= 250) && (b >= 250) && (g <= 5);
    }

private:
    // Helper methods to convert between legacy and new options
    static ConversionOptions convert_legacy_options(const Options& options);
    static PngOptions convert_png_options(const Options& options);
    static TgaOptions convert_tga_options(const Options& options);
    static BmpOptions convert_bmp_options(const Options& options);
};

// ============================================================================
// LEGACY VIEW STRUCTURES
// ============================================================================

// Forward declarations
struct ArtView;
struct ImageView;

/// @brief Legacy ArtView structure for zero-copy operations
struct ArtView {
    const uint8_t* art_data;           // Pointer to original ART memory
    size_t art_size;                   // Size of ART data
    const Palette* palette;            // Palette for color conversion
    ArtFile::Header header;            // ART file header
    std::vector<ArtFile::Tile> tiles;  // Tile metadata

    // On-demand ImageView creation
    size_t image_count() const {
        return tiles.size();
    }
    const ArtFile::Tile& get_tile(uint32_t tile_index) const {
        if (tile_index >= tiles.size()) {
            throw ArtException("Tile index out of range");
        }
        return tiles[tile_index];
    }

    uint32_t get_start_tile_index() const {
        return header.start_tile;
    }
    uint32_t get_end_tile_index() const {
        return header.end_tile;
    }
    uint32_t get_num_tiles() const {
        return header.num_tiles;
    }
};

/// @brief Legacy ImageView structure for zero-copy operations
struct ImageView {
    const ArtView* parent = nullptr;  // Parent ArtView
    uint32_t tile_index = 0;          // Index of this tile

    // Direct memory access
    const uint8_t* pixel_data() const;
    uint16_t width() const;
    uint16_t height() const;
    size_t size() const;

    // Animation data accessors
    uint32_t anim_frames() const;
    uint32_t anim_type() const;
    int8_t x_offset() const;
    int8_t y_offset() const;
    uint32_t anim_speed() const;
    uint32_t other_flags() const;

    // Image saving (conversion + write happens here)
    bool save_to_image(const std::filesystem::path& path, ImageFormat format,
                       ImageWriter::Options options = ImageWriter::Options()) const;
    bool save_to_png(const std::filesystem::path& path,
                     ImageWriter::Options options = ImageWriter::Options()) const;
    bool save_to_tga(const std::filesystem::path& path) const;
    bool save_to_bmp(const std::filesystem::path& path) const;

    // Image extraction to memory
    std::vector<uint8_t> extract_to_image(
        ImageFormat format, ImageWriter::Options options = ImageWriter::Options()) const;
    std::vector<uint8_t> extract_to_png(ImageWriter::Options options = ImageWriter::Options()) const;
    std::vector<uint8_t> extract_to_tga() const;
    std::vector<uint8_t> extract_to_bmp() const;

private:
    const ArtFile::Tile& require_tile() const;
};

// ============================================================================
// LEGACY EXTRACTOR API
// ============================================================================

/// @brief Legacy ExtractorAPI class (main entry point)
class ExtractorAPI {
public:
    ExtractorAPI();

    // File-based operations
    bool load_art_file(const std::filesystem::path& filename);
    bool load_palette_file(const std::filesystem::path& filename);

    // Memory-based operations
    bool load_art_from_memory(const uint8_t* data, size_t size);
    bool load_palette_from_memory(const uint8_t* data, size_t size);

    // Set default palettes
    void set_duke3d_default_palette();
    void set_blood_default_palette();

    // Extraction methods
    ExtractionResult extract_tile(uint32_t tile_index, ImageFormat format,
                                  ImageWriter::Options options = ImageWriter::Options());
    ExtractionResult extract_tile_png(uint32_t tile_index,
                                      ImageWriter::Options options = ImageWriter::Options());
    ExtractionResult extract_tile_tga(uint32_t tile_index,
                                      ImageWriter::Options options = ImageWriter::Options());
    ExtractionResult extract_tile_bmp(uint32_t tile_index,
                                      ImageWriter::Options options = ImageWriter::Options());

    // Batch extraction
    std::vector<ExtractionResult> extract_all_tiles(
        ImageFormat format, ImageWriter::Options options = ImageWriter::Options());
    std::vector<ExtractionResult> extract_all_tiles_png(
        ImageWriter::Options options = ImageWriter::Options());
    std::vector<ExtractionResult> extract_all_tiles_tga(
        ImageWriter::Options options = ImageWriter::Options());
    std::vector<ExtractionResult> extract_all_tiles_bmp(
        ImageWriter::Options options = ImageWriter::Options());

    // Accessors
    bool is_art_loaded() const;
    bool is_palette_loaded() const;
    uint32_t get_tile_count() const;

    // Zero-copy view access for parallel processing
    ArtView get_art_view() const;

    // Animation data handling
    bool write_animation_data(const std::string& art_file_path, const std::string& output_dir) const;
    std::string generate_animation_ini_content(const std::string& art_file_path) const;

private:
    std::unique_ptr<ArtFile> art_file_;
    std::unique_ptr<Palette> palette_;

    // Helper for animation data
    std::string get_animation_type_string(uint32_t anim_type) const;
    
    // Helper methods for conversion
    ExtractionResult create_extraction_result(
        const std::expected<std::vector<byte>, Error>& result,
        uint32_t tile_index,
        const std::string& format,
        const TileView& tile_view) const;
};

} // namespace art2img