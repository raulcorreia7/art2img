# art2img

Modern C++23 library and CLI tool for converting Build engine ART assets to PNG, TGA, and BMP formats.

## Quick Start

```bash
# Build (Linux native)
git clone https://github.com/raulcorreia7/art2img.git
cd art2img
make all

# Convert ART files
./build/linux_x64/cli/art2img_cli convert --input TILES.ART --output output/ --format png
```

## Features

- Modern C++23 API with `std::expected` error handling
- High-performance conversion with optional parallel processing
- CLI tool for batch processing
- Memory-safe with RAII and span-based views
- Legacy v1.x compatibility wrapper

## Usage

### C++ API

```cpp
#include <art2img/api.hpp>

auto palette = art2img::load_palette("PALETTE.DAT");
auto art = art2img::load_art_bundle("TILES.ART");
auto image = art2img::to_rgba(art2img::make_tile_view(*art, 0), *palette);
auto encoded = art2img::encode_png(art2img::image_view(*image));
art2img::write_binary_file("tile.png", *encoded);
```

### CLI

```bash
# Basic conversion
art2img convert --input TILES.ART --output output/ --format png

# With custom palette and parallel processing
art2img convert --input TILES.ART --palette PALETTE.DAT --output output/ --format png --jobs 4
```

## Requirements

- C++23 compiler (GCC 13+, Clang 16+, MSVC 19.35+)
- CMake 3.20+
- Git

## Cross-Platform Build System

The project supports building for multiple platforms from Linux:

```bash
# Get help with all available targets
make help

# Native builds
make all                    # Linux x64 (default)

# Windows cross-compilation (requires MinGW)
make windows-x64-mingw     # Windows 64-bit
make windows-x86-mingw     # Windows 32-bit  
make windows               # All Windows variants

# macOS cross-compilation (requires osxcross)
make macos-x64-osxcross    # macOS Intel
make macos-arm64-osxcross  # macOS Apple Silicon
make macos                  # All macOS variants

# Build everything
make all-platforms          # All supported platforms
```

### Prerequisites for Cross-Compilation

**Windows (MinGW):**
```bash
# Arch/Manjaro
sudo pacman -S mingw-w64-gcc

# Ubuntu/Debian
sudo apt-get install mingw-w64
```

**macOS (osxcross):**
```bash
git clone https://github.com/tpoechtrager/osxcross
cd osxcross && ./tools/gen_sdk_package.sh && ./build.sh
```

### Build Directory Structure

```
build/
├── linux_x64/      # Linux x64 binaries
├── windows_x64/    # Windows x64 binaries  
├── windows_x86/    # Windows x86 binaries
├── macos_x64/      # macOS Intel binaries
└── macos_arm64/    # macOS ARM64 binaries
```

## Traditional CMake Build

```bash
# Standard build
cmake -S . -B build
cmake --build build

# Debug with sanitizers
cmake -S . -B build-debug -DCMAKE_BUILD_TYPE=Debug -DENABLE_ASAN=ON
cmake --build build-debug
```

## Testing

```bash
cd build && ctest --output-on-failure
```

## Documentation

- [Architecture](docs/plan/architecture.md)
- [Migration Guide](docs/plan/migration_guide.md)
- [Format Specifications](docs/specs/)

## Dependencies

- **stb** - Image encoding
- **doctest** - Testing framework
- **CLI11** - Command-line parsing
- **fmt** - String formatting

All dependencies are fetched automatically via CPM.

## License

See [LICENSE](LICENSE) for license information.