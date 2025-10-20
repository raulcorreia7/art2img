# Migration Guide: art2img v1.x to v2.0

This guide helps users migrate from the legacy art2img v1.x API to the new v2.0 C++23 architecture.

## Overview of Changes

The art2img v2.0 refactor introduces a completely new API design:

- **Language**: C++23 (was C++17)
- **Error Handling**: `std::expected<T, Error>` throughout (was exceptions/bool returns)
- **Design**: Stateless functions + plain structs (was class-based OOP)
- **Build System**: Modern CMake with CPM package management
- **API Surface**: Functional pipeline with modular headers

## Quick Start

### New Include Pattern

```cpp
// Old v1.x
#include <art2img/extractor_api.hpp>
#include <art2img/art_file.hpp>

// New v2.0
#include <art2img/api.hpp>  // Single include for all modules
// Or specific modules:
#include <art2img/palette.hpp>
#include <art2img/art.hpp>
#include <art2img/convert.hpp>
```

### Basic Migration Example

```cpp
// Old v1.x approach
try {
    ExtractorAPI extractor;
    extractor.load_palette("PALETTE.DAT");
    extractor.extract_tiles("TILES.ART", "output/");
} catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
}

// New v2.0 approach
auto palette_result = art2img::load_palette("PALETTE.DAT");
if (!palette_result) {
    std::cerr << "Palette error: " << palette_result.error().message << std::endl;
    return 1;
}

auto art_result = art2img::load_art_bundle("TILES.ART");
if (!art_result) {
    std::cerr << "ART error: " << art_result.error().message << std::endl;
    return 1;
}

// Process tiles...
```

## API Mapping

### Palette Operations

| v1.x | v2.0 |
|------|------|
| `Palette::loadFromFile()` | `load_palette(path)` |
| `Palette::getData()` | `Palette::data` field |
| Exception on error | `std::expected<Palette, Error>` |

### ART File Operations

| v1.x | v2.0 |
|------|------|
| `ArtFile::load()` | `load_art_bundle(path)` |
| `ArtFile::getTile()` | `make_tile_view(art_data, index)` |
| `ArtFile::getTileCount()` | `art_data.tiles.size()` |
| Exception on error | `std::expected<ArtData, Error>` |

### Image Conversion

| v1.x | v2.0 |
|------|------|
| `ImageProcessor::convertToRGBA()` | `to_rgba(tile_view, palette, options)` |
| `ImageWriter::writePNG()` | `encode_png(image_view, options)` |
| Manual memory management | RAII with `Image`/`ImageView` |

## Error Handling Migration

### v1.x (Exceptions)
```cpp
try {
    auto art = ArtFile::load("tiles.art");
    // Process...
} catch (const InvalidArtException& e) {
    std::cerr << "Invalid ART: " << e.what() << std::endl;
}
```

### v2.0 (std::expected)
```cpp
auto art_result = load_art_bundle("tiles.art");
if (!art_result) {
    auto error = art_result.error();
    std::cerr << "Invalid ART: " << error.message << std::endl;
    return 1;
}
auto& art = art_result.value();
// Process...
```

## Build System Migration

### CMake Changes

```cmake
# Old v1.x
find_package(art2img REQUIRED)
target_link_libraries(myapp art2img)

# New v2.0
find_package(art2img CONFIG REQUIRED)
target_link_libraries(myapp art2img::core)

# Optional: Legacy compatibility
find_package(art2img CONFIG REQUIRED)
target_link_libraries(myapp art2img::core art2img::legacy)
```

### Legacy Compatibility

To maintain compatibility with existing code during migration:

```cmake
# Enable legacy wrapper
set(ART2IMG_ENABLE_LEGACY ON)
```

This provides the old API surface that internally uses the new implementation.

## CLI Migration

### Command Line Changes

```bash
# Old v1.x
art2img-extractor --input tiles.art --output output/ --format png

# New v2.0
art2img convert --input tiles.art --output output/ --format png
```

### CLI Options

The new CLI provides similar functionality with updated option names:

| v1.x Option | v2.0 Option |
|-------------|-------------|
| `--input` | `--input` (same) |
| `--output` | `--output` (same) |
| `--format` | `--format` (same) |
| `--palette` | `--palette` (same) |
| `--verbose` | `--verbose` (same) |
| `--threads` | `--jobs` |

## Performance Considerations

### Memory Management
- v2.0 uses RAII consistently - no manual memory management needed
- Span-based views avoid unnecessary copies
- Contiguous memory layout improves cache performance

### Error Handling
- `std::expected` has zero overhead for success cases
- No exception handling overhead in the core pipeline

### Threading
- v2.0 provides built-in parallel processing options
- Thread pool implementation is optional and configurable

## Testing Migration

### Unit Tests
```cpp
// Old v1.x style
TEST_CASE("ArtFile loading") {
    REQUIRE_NOTHROW(ArtFile::load("test.art"));
}

// New v2.0 style
TEST_CASE("ART bundle loading") {
    auto result = load_art_bundle("test.art");
    REQUIRE(result.has_value());
}
```

## Troubleshooting

### Common Issues

1. **Missing Headers**: Use `#include <art2img/api.hpp>` for all modules
2. **Link Errors**: Link against `art2img::core` instead of `art2img`
3. **Template Errors**: Ensure C++23 standard is enabled
4. **Runtime Errors**: Check `std::expected` return values instead of using try/catch

### Getting Help

- Check the [Architecture Overview](architecture.md) for detailed design information
- Review the [Implementation Tasks](tasks.md) for technical details
- Examine the legacy wrapper code in `src/legacy_api.cpp` for compatibility examples

## Timeline

- **Phase 1**: New API available alongside legacy (current state)
- **Phase 2**: Legacy API deprecated but still supported
- **Phase 3**: Legacy API removed (future major version)

## Conclusion

The v2.0 architecture provides:
- Better performance through modern C++23 features
- More robust error handling with `std::expected`
- Cleaner separation of concerns with functional design
- Easier testing and maintenance

The migration path is designed to be gradual, with the legacy wrapper providing compatibility during the transition period.