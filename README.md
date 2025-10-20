# art2img

A modern C++23 library and CLI tool for extracting and converting ART format assets (from Build engine games) to modern image formats.

## Features

- **Modern C++23 Architecture**: Stateless functions with plain structs
- **Robust Error Handling**: `std::expected<T, Error>` throughout the API
- **High Performance**: Optimized memory layout and optional parallel processing
- **Comprehensive Format Support**: PNG, TGA, BMP output formats
- **CLI Tool**: Full-featured command-line interface
- **Legacy Compatibility**: Optional wrapper for v1.x API migration

## Quick Start

### Installation

```bash
# Clone the repository
git clone https://github.com/raulcorreia7/art2img.git
cd art2img

# Configure and build
cmake -S . -B build -DART2IMG_ENABLE_LEGACY=ON
cmake --build build

# Run tests (optional)
cd build && ctest
```

### Basic Usage

#### C++ API

```cpp
#include <art2img/api.hpp>

int main() {
    // Load palette
    auto palette = art2img::load_palette("PALETTE.DAT");
    if (!palette) {
        std::cerr << "Palette error: " << palette.error().message << std::endl;
        return 1;
    }

    // Load ART bundle
    auto art = art2img::load_art_bundle("TILES.ART");
    if (!art) {
        std::cerr << "ART error: " << art.error().message << std::endl;
        return 1;
    }

    // Convert first tile to RGBA
    auto tile_view = art2img::make_tile_view(*art, 0);
    auto image = art2img::to_rgba(tile_view, *palette);
    if (!image) {
        std::cerr << "Conversion error: " << image.error().message << std::endl;
        return 1;
    }

    // Encode as PNG
    auto encoded = art2img::encode_png(art2img::image_view(*image));
    if (!encoded) {
        std::cerr << "Encoding error: " << encoded.error().message << std::endl;
        return 1;
    }

    // Save to file
    auto write_result = art2img::write_binary_file("tile_000.png", *encoded);
    if (!write_result) {
        std::cerr << "Write error: " << write_result.error().message << std::endl;
        return 1;
    }

    return 0;
}
```

#### CLI Tool

```bash
# Convert all tiles from an ART file to PNG
art2img convert --input TILES.ART --output output/ --format png

# Specify custom palette and enable parallel processing
art2img convert \
    --input TILES.ART \
    --palette PALETTE.DAT \
    --output output/ \
    --format png \
    --jobs 4

# Apply shading and transparency fixes
art2img convert \
    --input TILES.ART \
    --output output/ \
    --format tga \
    --shade-index 1 \
    --fix-transparency
```

## Directory Structure

```
├── include/art2img/     # Public headers (C++23)
│   ├── api.hpp          # Single include for all modules
│   ├── types.hpp        # Type definitions and constants
│   ├── error.hpp        # Error handling utilities
│   ├── palette.hpp      # Palette loading and processing
│   ├── art.hpp          # ART file loading
│   ├── convert.hpp      # Tile conversion to RGBA
│   ├── encode.hpp       # Image encoding (PNG/TGA/BMP)
│   ├── io.hpp           # File I/O utilities
│   └── legacy_api.hpp   # Legacy compatibility wrapper
├── src/                 # Implementation files
├── cli/                 # Command-line interface
├── tests/               # Test suite
├── cmake/               # Build configuration
├── docs/plan/           # Architecture and planning docs
├── docs/specs/          # Format specifications
└── repository/legacy/   # Previous implementation (preserved)
```

## Building

### Prerequisites

- C++23 compliant compiler (GCC 13+, Clang 16+, MSVC 19.35+)
- CMake 3.20 or later
- Git (for fetching dependencies)

### Build Options

```bash
# Basic build
cmake -S . -B build -DART2IMG_ENABLE_LEGACY=OFF
cmake --build build

# Build with legacy compatibility wrapper
cmake -S . -B build -DART2IMG_ENABLE_LEGACY=ON
cmake --build build

# Debug build with sanitizers
cmake -S . -B build-debug \
    -DCMAKE_BUILD_TYPE=Debug \
    -DART2IMG_ENABLE_LEGACY=ON \
    -DENABLE_ASAN=ON \
    -DENABLE_UBSAN=ON \
    -DENABLE_LEAK_SANITIZER=ON
cmake --build build-debug
```

### Build Targets

- `art2img_core`: Core library
- `art2img_cli`: Command-line tool
- `art2img_tests`: Test suite (requires `-DBUILD_TESTING=ON`)

## Testing

```bash
# Run all tests
cd build && ctest --output-on-failure

# Run specific test suites
ctest -R "palette"
ctest -R "art"
ctest -R "convert"
ctest -R "encode"

# Run CLI integration tests
cd build && ./bin/art2img --help
```

## Migration from v1.x

If you're migrating from the legacy v1.x API, see the [Migration Guide](docs/plan/migration_guide.md) for detailed instructions. The legacy compatibility wrapper can be enabled with:

```cmake
set(ART2IMG_ENABLE_LEGACY ON)
```

## Documentation

- [Architecture Overview](docs/plan/architecture.md) - Detailed design documentation
- [Migration Guide](docs/plan/migration_guide.md) - v1.x to v2.0 migration
- [Implementation Tasks](docs/plan/tasks.md) - Development roadmap
- [Format Specifications](docs/specs/) - ART, PALETTE, and output format docs

## Dependencies

- **stb**: Image encoding (single-header library)
- **doctest**: Testing framework (development only)
- **CLI11**: Command-line parsing (CLI only)
- **fmt**: String formatting (CLI only)

All dependencies are automatically fetched via CPM during build.

## License

See [`repository/legacy/LICENSE`](repository/legacy/LICENSE) for license information.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Add tests for new functionality
5. Ensure all tests pass
6. Submit a pull request

## Architecture Highlights

### Error Handling

The new API uses `std::expected<T, Error>` for robust error handling without exceptions:

```cpp
auto result = art2img::load_palette("palette.dat");
if (!result) {
    // Handle error
    std::cerr << "Error: " << result.error().message << std::endl;
    return;
}
// Use result.value()
```

### Memory Safety

- RAII-based resource management
- Span-based views for zero-copy operations
- Bounds checking throughout the pipeline
- No manual memory management required

### Performance

- Contiguous memory layout for better cache locality
- Optional parallel processing for batch operations
- Minimal allocation overhead
- Efficient column-major to row-major conversion

### Modularity

The library is organized into focused modules that can be used independently:

```cpp
// Use only palette functionality
#include <art2img/palette.hpp>

// Use only ART loading
#include <art2img/art.hpp>

// Or include everything
#include <art2img/api.hpp>