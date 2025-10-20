# art2img API Iteration Plan

This document captures the evolution of the art2img public API design through multiple iterations, focusing on creating a simple, powerful, memory-first API for modders.

---

## Iteration Summary

### Iteration 1: Initial Assessment
- **Problem:** Current API was over-engineered with 8 separate modules
- **Issues:** Excessive modularity, verbose type system, complex option hierarchies
- **Goal:** Simplify while maintaining power and memory-first design

### Iteration 2: Human-First Design
- **Approach:** Complete access to all data fields, human-readable naming
- **Features:** Full manipulation APIs, analysis tools, advanced filtering
- **Issue:** Still too complex for 80% of use cases

### Iteration 3: Pragmatic 80% Solution
- **Focus:** What most modders actually need
- **Simplification:** Reduced to essential functions, single header include
- **Trade-off:** Removed advanced features for simplicity

### Iteration 4: TileView-First Design
- **Correction:** User feedback - use TileView instead of Tile objects
- **Benefit:** Non-owning views, memory efficiency, zero-copy access
- **Refinement:** Focused on loading and viewing, not manipulation

### Iteration 5: Missing Fields Analysis
- **Gap Analysis:** Reviewed ART format specification for missing fields
- **Additions:** Complete animation data (speed, offsets, proper type enum)
- **Completion:** Added version, tile ID ranges, proper ID mapping

### Iteration 6: Thread Safety Design
- **Requirement:** Memory-first, thread-safe API design
- **Approach:** Immutable inputs, read-only views, pure functions
- **Guarantee:** All functions safe to call from multiple threads

### Iteration 7: Function-Based API Design
- **Consolidation:** Single function per operation type
- **Pattern:** `encode(image, options, format)` instead of `encode_png()`
- **Benefit:** Consistent API surface, easier to learn

### Iteration 8: Separated Output Control
- **Final Refinement:** Separate folder and filename parameters
- **Control:** User has complete control over output structure
- **Clarity:** No ambiguity about path composition

---

## Implementation Decision: Unified Structure

### Problem: ArtData vs ArtFile Duplication

The original plan proposed both `ArtData` (current implementation) and `ArtFile` (planned API), creating unnecessary complexity and potential confusion.

### Solution: Single Unified Structure

**Decision**: Rename existing `ArtData` to `ArtFile` and enhance it with planned API methods.

**Benefits:**
- ✅ **No duplication** - single source of truth
- ✅ **Zero breaking changes** - existing code works with rename
- ✅ **Memory-safe** - maintains current RAII approach
- ✅ **Simple implementation** - just rename and add methods
- ✅ **Perfect parity** - matches planned API exactly

**Changes Required:**
1. **Rename**: `ArtData` → `ArtFile` (global search/replace)
2. **Add**: Missing methods (`header()`, `tiles()`, iterators)
3. **Enhance**: `TileView` with convenience accessors
4. **Create**: Unified `art2img.hpp` header
5. **Add**: Template support for filename generation

### Memory Safety Guarantees

**RAII Compliance:**
- **Owning containers**: `std::vector` for automatic memory management
- **Non-owning views**: `std::span` for safe, bounds-checked access
- **Exception safety**: Automatic cleanup on exceptions
- **No raw pointers**: All memory managed through STL

**Lifetime Safety:**
- **Clear ownership**: `ArtFile` owns all data, `TileView` is non-owning
- **Move semantics**: Efficient transfer of ownership
- **Thread safety**: All functions use immutable inputs

---

## Final API Design

### Core Principles
1. **Memory-First:** All operations work on memory spans
2. **Thread-Safe:** Pure functions with immutable inputs
3. **TileView-Based:** Non-owning views for efficiency
4. **Function-Based:** Single function per operation type
5. **Separated Output:** Folder and filename control are independent
6. **Memory-Safe:** RAII with std::vector/std::span, no raw pointers
7. **Unified Structure:** Single ArtFile type (no duplication with ArtData)

### API Surface

```cpp
namespace art2img {
    // Core Types
    enum class ImageFormat { png, tga, bmp };
    struct Error { /* error handling */ };
    
    struct TileView {
        // Core tile data
        u16 width = 0;
        u16 height = 0;
        u32 id = 0;                  // Tile ID for identification
        u8_span pixels;               // Column-major palette indices
        u8_span remap;                // Optional remap data
        TileAnimation animation;         // Animation information
        
        // Convenience accessors
        std::span<const u8> pixel_data() const noexcept { return pixels; }
        std::span<const u8> remap_data() const noexcept { return remap; }
        TileAnimation animation_data() const noexcept { return animation; }
        
        // Thread-safe read-only methods
        std::expected<u8, Error> get_pixel(u16 x, u16 y) const noexcept;
        bool is_empty() const noexcept { return width == 0 || height == 0; }
        bool is_animated() const noexcept { return animation.frame_count > 0; }
        std::string dimensions_string() const;
        
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
        
        // Accessors
        std::span<const u8> palette_data() const noexcept;
        std::span<const u8> shade_data() const noexcept;
        std::span<const u8> translucent_data() const noexcept;
        
        // Color conversion
        std::array<u8, 3> get_rgb(u8 index) const;
        u32 get_rgba(u8 index) const;
        color::Color get_color(u8 index) const;
        
        // Validation
        constexpr bool has_shade_tables() const noexcept;
        constexpr bool has_translucent_map() const noexcept;
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
    
    struct ArtFile {
        struct Header {
            u32 version;
            u32 total_tiles;
            u32 first_tile_id;
            u32 last_tile_id;
        };
        
        // Core data fields (owning)
        u32 version = 0;
        u32 tile_start = 0;           // first_tile_id
        u32 tile_end = 0;             // last_tile_id
        std::vector<u8> pixels;        // Owning buffer (column-major)
        std::vector<u8> remaps;        // Owning buffer (optional)
        std::vector<TileView> tiles;   // Non-owning views
        std::vector<u32> tile_ids;
        
        // Header access
        Header header() const noexcept;
        
        // Thread-safe access methods
        std::optional<TileView> get_tile(u32 tile_id) const;           // Get tile by actual tile ID
        std::optional<TileView> get_tile_by_index(size_t index) const; // Get tile by array index
        std::span<const TileView> tiles() const noexcept;                // **Get ALL tiles as span**
        size_t tile_count() const noexcept;
        
        // Iterator support
        auto begin() const noexcept { return tiles().begin(); }
        auto end() const noexcept { return tiles().end(); }
        
        // Raw data access
        std::span<const std::byte> raw_data() const noexcept;
        
        // Validation
        constexpr bool is_valid() const noexcept;
    };
}

// Loading Functions (Thread-Safe, Memory-Safe)
namespace art2img {
    // Core loading functions (unified from ArtData)
    std::expected<ArtFile, Error> load_art(
        const std::filesystem::path& path, 
        PaletteHint hint = PaletteHint::none);
    std::expected<ArtFile, Error> load_art(
        std::span<const std::byte> data, 
        PaletteHint hint = PaletteHint::none);
    
    // Convenience wrappers
    std::expected<ArtFile, Error> load_art_file(const std::filesystem::path& path);
    std::expected<Palette, Error> load_palette_file(const std::filesystem::path& path);
    
    // Existing palette loading (unchanged)
    std::expected<Palette, Error> load_palette(std::span<const std::byte> data);
}

// Processing Functions (Thread-Safe, Function-Based, Memory-Safe)
namespace art2img {
    struct ConvertOptions {
        bool fix_transparency = true;
        std::optional<u8> shade_index = std::nullopt;
        bool premultiply_alpha = false;
        float gamma = 1.0f;
        
        // Map to existing ConversionOptions internally
        ConversionOptions to_legacy() const;
    };
    
    // NEW: Unified convert function over existing to_rgba()
    std::expected<Image, Error> convert(
        const TileView& tile,
        const Palette& palette,
        const ConvertOptions& options = {},
        ImageFormat target_format = ImageFormat::png
    );
    
    // NEW: Batch conversion
    std::expected<std::vector<Image>, Error> convert_all(
        const ArtFile& art_file,
        const Palette& palette,
        const ConvertOptions& options = {},
        ImageFormat target_format = ImageFormat::png
    );
    
    // EXISTING: Keep to_rgba() for backward compatibility
    std::expected<Image, Error> to_rgba(
        const TileView& tile, 
        const Palette& palette, 
        const ConversionOptions& = {});
}

// Encoding Functions (Thread-Safe, Function-Based)
namespace art2img {
    struct EncodeOptions {
        bool include_alpha = true;
        bool flip_vertically = false;
        
        std::variant<
            std::monostate,
            struct { u8 compression = 6; bool filters = true; },  // PNG
            struct { bool rle = false; },                       // TGA
            std::monostate                                     // BMP
        > format_specific;
        
        static EncodeOptions png(u8 compression = 6, bool filters = true);
        static EncodeOptions tga(bool rle = false);
        static EncodeOptions bmp();
    };
    
    std::expected<std::vector<std::byte>, Error> encode(
        const ImageView& image,
        const EncodeOptions& options = {},
        ImageFormat format
    );
}

// Export Functions (Thread-Safe, Separated Output, Memory-Safe)
namespace art2img {
    struct ExportOptions {
        ImageFormat format = ImageFormat::png;
        bool fix_transparency = true;
        bool overwrite_existing = true;
        bool create_subdirectories = false;
        bool export_metadata = false;
    };
    
    // NEW: Separated folder/filename parameters (planned API)
    std::expected<std::monostate, Error> export_tile(
        const TileView& tile,
        const Palette& palette,
        const std::filesystem::path& folder,     // Folder only
        const std::string& filename,            // Filename only
        const ExportOptions& options = {},
        ImageFormat format = ImageFormat::png
    );
    
    // NEW: Template-based batch export
    std::expected<std::monostate, Error> export_tiles(
        std::span<const TileView> tiles,
        const Palette& palette,
        const std::filesystem::path& folder,     // Folder only
        const std::string& filename_template,     // Template like "tile_{id:04d}"
        const ExportOptions& options = {},
        ImageFormat format = ImageFormat::png
    );
    
    // NEW: Export entire ART file
    std::expected<std::monostate, Error> export_art_file(
        const ArtFile& art_file,
        const Palette& palette,
        const std::filesystem::path& folder,     // Folder only
        const std::string& filename_template,     // Template
        const ExportOptions& options = {},
        ImageFormat format = ImageFormat::png
    );
    
    // EXISTING: Keep current export functions for compatibility
    std::expected<ExportResult, Error> export_tile(
        const TileView& tile,
        const Palette& palette,
        const ExportOptions& options);
}
```

### Unified Header

**New: `include/art2img.hpp`**

Single entry point for all art2img functionality:

```cpp
/// @file art2img.hpp
/// @brief Unified header for art2img public API

// Core types and error handling
#include "art2img/types.hpp"
#include "art2img/error.hpp"

// Core modules  
#include "art2img/palette.hpp"
#include "art2img/art.hpp"           // Contains unified ArtFile
#include "art2img/convert.hpp"
#include "art2img/encode.hpp"
#include "art2img/export.hpp"
#include "art2img/io.hpp"

// Version information
#include "art2img/api.hpp"
```

### Usage Examples

**Basic Usage (80% of cases):**
```cpp
#include <art2img/art2img.hpp>

// Load from memory
auto art = art2img::load_art_file("TILES.ART");
auto palette = art2img::load_palette_file("PALETTE.DAT");

if (art && palette) {
    // Get tile by ID
    auto tile = art->get_tile(100);
    if (tile) {
        // Convert directly to format
        auto image = art2img::convert(*tile, *palette, {}, ImageFormat::png);
        
        // Export with separated folder/filename
        auto result = art2img::export_tile(
            *tile, *palette,
            "output/",                    // Folder only
            "my_tile.png",                // Filename only
            {},                            // Default options
            ImageFormat::png
        );
    }
    
    // Export all with template
    auto batch_result = art2img::export_art_file(
        *art, *palette,
        "exported_tiles/",              // Folder only
        "texture_{id:04d}",          // Template
        {},                            // Default options
        ImageFormat::png
    );
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

### Requirements Met
1. ✅ **Memory-First:** All operations work on memory spans
2. ✅ **Thread-Safe:** All functions are pure with immutable inputs
3. ✅ **TileView-Based:** Non-owning views for zero-copy access
4. ✅ **Function-Based:** Single function per operation type
5. ✅ **Separated Output:** Folder and filename are independent parameters
6. ✅ **Complete ART Coverage:** All format fields accessible
7. ✅ **Simple Learning Curve:** Most use cases require 2-3 function calls
8. ✅ **Power User Support:** Advanced features still accessible
9. ✅ **Memory-Safe:** RAII with std::vector/std::span, no raw pointers
10. ✅ **Unified Structure:** Single ArtFile type (no duplication with ArtData)
11. ✅ **Backward Compatible:** Existing code continues to work during transition

### API Surface Summary
- **Single Header:** `#include <art2img/art2img.hpp>`
- **Core Functions:** ~15 total functions vs 50+ in original design
- **Complete Coverage:** All ART format fields exposed
- **Thread Safety:** Guaranteed by design
- **Memory Efficiency:** Zero-copy TileView access
- **Output Control:** Complete folder and filename separation
- **Memory Safety:** RAII with std library, automatic cleanup
- **Unified API:** Single ArtFile structure instead of ArtData duplication

### Implementation Strategy
1. **Phase 1 - Core Unification:** Rename ArtData → ArtFile, add missing methods
2. **Phase 2 - Function Mapping:** Add convert() wrapper, rename loading functions  
3. **Phase 3 - Export Enhancement:** Add separated folder/filename overloads
4. **Phase 4 - Header & Documentation:** Create unified art2img.hpp
5. **Phase 5 - Migration Path:** Temporary aliases for backward compatibility

This revised design achieves the goal of a simple, powerful, organic API that serves 80% of modders with minimal complexity while providing complete access for power users, with unified structures and guaranteed memory safety through RAII.