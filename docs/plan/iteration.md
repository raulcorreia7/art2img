# art2img API Design

This document describes the art2img public API design, focusing on a simple, powerful, memory-first API for modders.

---

## API Design Principles

The art2img API follows these core design principles:

1. **Memory-First:** All operations work on memory spans for maximum efficiency
2. **Thread-Safe:** Pure functions with immutable inputs enable safe parallel processing
3. **TileView-Based:** Non-owning views provide zero-copy access to tile data
4. **Function-Based:** Single function per operation type creates a consistent API surface
5. **Memory-Safe:** RAII with std::vector/std::span eliminates raw pointer management
6. **Modern C++:** Uses contemporary C++ features for clean, expressive code

---

## API Architecture

### Core Data Structures

The art2img API is built around three primary data structures:

**ArtData**: The primary ART file container that owns all file data and metadata
- Contains version information, tile ID ranges, and pixel data
- Provides thread-safe access methods for tile retrieval
- Manages memory ownership through RAII containers

**TileView**: Non-owning view for efficient tile data access
- Provides zero-copy access to tile dimensions and pixel data
- Includes animation metadata and validation methods
- Enables safe parallel processing without memory overhead

**Palette**: Complete palette data with shade tables and translucency maps
- Owns palette data and provides color conversion utilities
- Includes shade tables for advanced lighting effects
- Supports translucency mapping for special rendering effects

### Memory Safety Guarantees

**RAII Compliance:**
- **Owning containers**: `std::vector` for automatic memory management
- **Non-owning views**: `std::span` for safe, bounds-checked access
- **Exception safety**: Automatic cleanup on exceptions
- **No raw pointers**: All memory managed through STL

**Lifetime Safety:**
- **Clear ownership**: `ArtData` owns all data, `TileView` is non-owning
- **Move semantics**: Efficient transfer of ownership
- **Thread safety**: All functions use immutable inputs

---

## API Reference

### Core Principles
1. **Memory-First:** All operations work on memory spans
2. **Thread-Safe:** Pure functions with immutable inputs
3. **TileView-Based:** Non-owning views for efficiency
4. **Function-Based:** Single function per operation type
5. **Memory-Safe:** RAII with std::vector/std::span, no raw pointers
6. **Modern Structure:** ArtData type as the primary container

### Current API Surface

```cpp
namespace art2img {
    // Core Types
    enum class ImageFormat { png, tga, bmp };
    struct Error { /* error handling */ };
    
    struct TileView {
        // Core tile data
        u16 width = 0;
        u16 height = 0;
        u8_span pixels;               // Column-major palette indices
        u8_span remap;                // Optional remap data
        TileAnimation animation;         // Animation information
        
        // Validation
        constexpr bool is_valid() const noexcept;
        constexpr bool has_remap() const noexcept;
        constexpr std::size_t pixel_count() const noexcept;
        constexpr bool has_valid_pixel_data() const noexcept;
    };
    
    struct Palette {
        // Owning palette data (RAII)
        std::array<u8, constants::PALETTE_DATA_SIZE> data{};
        std::uint16_t shade_table_count = 0;
        std::vector<u8> shade_tables{};
        std::array<u8, constants::TRANSLUCENT_TABLE_SIZE> translucent_map{};
        
        // Accessors and conversion functions
        std::span<const u8> palette_data() const noexcept;
        std::span<const u8> shade_data() const noexcept;
        std::span<const u8> translucent_data() const noexcept;
        u32 palette_entry_to_rgba(const Palette&, u8 index);
        // ... other conversion functions
    };
    
    struct Image {
        // Owning image data (RAII)
        std::vector<u8> data;  // RGBA pixels, row-major
        u16 width = 0;
        u16 height = 0;
        std::size_t stride = 0;
        
        // Accessors
        std::span<const u8> pixels() const noexcept;
        std::span<u8> pixels() noexcept;
        
        // Validation
        constexpr bool is_valid() const noexcept;
        constexpr std::size_t pixel_count() const noexcept;
    };
    
    struct ArtData {
        // Core data fields (owning)
        u32 version = 0;
        u32 tile_start = 0;
        u32 tile_end = 0;
        std::vector<u8> pixels;        // Owning buffer (column-major)
        std::vector<u8> remaps;        // Owning buffer (optional)
        std::vector<TileView> tiles;   // Non-owning views
        std::vector<u32> tile_ids;
        
        // Thread-safe access methods
        std::optional<TileView> get_tile(std::size_t index) const noexcept;
        std::optional<TileView> get_tile_by_id(u32 tile_id) const noexcept;
        constexpr std::size_t tile_count() const noexcept;
        constexpr bool is_valid() const noexcept;
    };
}

// Loading Functions (Thread-Safe, Memory-Safe)
namespace art2img {
    // Core loading functions (current implementation)
    std::expected<ArtData, Error> load_art_bundle(
        const std::filesystem::path& path, 
        PaletteHint hint = PaletteHint::none);
    std::expected<ArtData, Error> load_art_bundle(
        std::span<const std::byte> data, 
        PaletteHint hint = PaletteHint::none);
    
    // Tile view creation
    std::optional<TileView> make_tile_view(const ArtData& art_data, std::size_t index);
    std::optional<TileView> make_tile_view_by_id(const ArtData& art_data, u32 tile_id);
    
    // Palette loading
    std::expected<Palette, Error> load_palette(const std::filesystem::path& path);
    std::expected<Palette, Error> load_palette(std::span<const std::byte> data);
}

// Processing Functions (Thread-Safe, Function-Based, Memory-Safe)
namespace art2img {
    struct ConversionOptions {
        bool apply_lookup = false;
        bool fix_transparency = true;
        bool premultiply_alpha = false;
        bool matte_hygiene = false;
        u8 shade_index = 0;
    };
    
    // Current convert function
    std::expected<Image, Error> to_rgba(
        const TileView& tile, 
        const Palette& palette, 
        const ConversionOptions& = {});
    
    // Image view creation
    ImageView image_view(const Image& image);
    
    // Column-major conversion utilities
    std::expected<std::monostate, Error> convert_column_to_row_major(
        const TileView& tile, std::span<u8> destination);
    std::expected<u8, Error> get_pixel_column_major(const TileView& tile, u16 x, u16 y);
}

// Encoding Functions (Thread-Safe, Function-Based)
namespace art2img {
    // Current encoding function
    std::expected<std::vector<std::byte>, Error> encode_image(
        ImageView image, 
        ImageFormat format
    );
    
    // IO functions
    std::expected<std::vector<std::byte>, Error> read_binary_file(const std::filesystem::path& path);
    std::expected<std::monostate, Error> write_binary_file(const std::filesystem::path& path, std::span<const std::byte> data);
}

// Animation Export Functions (Thread-Safe, Memory-Safe)
namespace art2img {
    struct AnimationExportConfig {
        std::filesystem::path output_dir = ".";
        std::string base_name = "tile";
        bool include_non_animated = true;
        bool generate_ini = true;
        std::string ini_filename = "animdata.ini";
        ImageFormat image_format = ImageFormat::png;
        bool include_image_references = true;
    };
    
    // Current animation export function
    std::expected<std::monostate, Error> export_animation_data(
        const ArtData& art_data, 
        const AnimationExportConfig& config = {}
    );
    
    std::string get_animation_type_string(TileAnimation::Type type);
}
```

### Header Structure

**Primary Entry Point: `include/art2img/api.hpp`**

The main entry point for art2img functionality:

```cpp
/// @file api.hpp
/// @brief Barrel include for the art2img public API

// Core types and error handling
#include "error.hpp"
#include "types.hpp"

// Core modules
#include "art.hpp"
#include "convert.hpp"
#include "encode.hpp"
#include "io.hpp"
#include "palette.hpp"
```

This header provides access to all art2img functionality through a single include.

### Usage Examples

**Basic Usage:**
```cpp
#include <art2img/api.hpp>

// Load from files
auto art = art2img::load_art_bundle("TILES.ART");
auto palette = art2img::load_palette("PALETTE.DAT");

if (art && palette) {
    // Get tile by index
    auto tile = art2img::make_tile_view(*art, 0);
    if (tile) {
        // Convert to RGBA
        auto image = art2img::to_rgba(*tile, *palette);
        
        // Create image view
        auto view = art2img::image_view(*image);
        
        // Encode to PNG
        auto encoded = art2img::encode_image(view, art2img::ImageFormat::png);
        
        // Write to file
        art2img::write_binary_file("tile.png", *encoded);
    }
    
    // Export animation data
    auto anim_result = art2img::export_animation_data(*art);
}
```

**Thread-Safe Parallel Processing:**
```cpp
// User controls parallelism
std::vector<std::thread> threads;
for (size_t t = 0; t < std::thread::hardware_concurrency(); ++t) {
    threads.emplace_back([&, t]() {
        const std::string folder = std::format("output_thread_{}/", t);
        const size_t start = t * tiles_per_thread;
        const size_t end = std::min(start + tiles_per_thread, art->tile_count());
        
        for (size_t i = start; i < end; ++i) {
            auto tile = art->get_tile_by_index(i);
            if (tile) {
                art2img::export_tile(*tile, *palette, folder, 
                    std::format("tile_{:04d}.png", i), {}, ImageFormat::png);
            }
        }
    });
}
for (auto& thread : threads) thread.join();
```

---

## Acceptance Criteria

### API Requirements
1. [✓] **Memory-First:** All operations work on memory spans
2. [✓] **Thread-Safe:** All functions are pure with immutable inputs
3. [✓] **TileView-Based:** Non-owning views for zero-copy access
4. [✓] **Function-Based:** Single function per operation type
5. [✓] **Complete ART Coverage:** All format fields accessible
6. [✓] **Simple Learning Curve:** Most use cases require 3-4 function calls
7. [✓] **Power User Support:** Advanced features like animation export available
8. [✓] **Memory-Safe:** RAII with std::vector/std::span, no raw pointers
9. [✓] **Modern Structure:** ArtData type provides clean, consistent API

### API Surface Summary
- **Single Entry Point:** `#include <art2img/api.hpp>` provides all functionality
- **Core Functions:** ~20 total functions covering all operations
- **Complete Coverage:** All ART format fields exposed
- **Thread Safety:** Guaranteed by design
- **Memory Efficiency:** Zero-copy TileView access
- **Memory Safety:** RAII with std library, automatic cleanup
- **Modern Structure:** ArtData provides consistent, well-documented API

### Implementation Status
The implementation is **complete and production-ready**:
- [✓] All ART file loading and processing works
- [✓] Animation export functionality implemented
- [✓] CLI tool with full feature set
- [✓] Cross-platform build system
- [✓] Thread-safe operations
- [✓] Comprehensive test coverage
- [✓] Complete documentation

### Quality Assurance
- **Memory Safety:** No raw pointers, automatic cleanup through RAII
- **Thread Safety:** All functions use immutable inputs
- **Error Handling:** Comprehensive error reporting through std::expected
- **Performance:** Zero-copy access patterns for optimal speed
- **Maintainability:** Clean, modular code structure with clear separation of concerns

The art2img API provides a modern, efficient, and safe interface for working with ART files, meeting all requirements for both casual users and power users.