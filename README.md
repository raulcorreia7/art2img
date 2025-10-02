# art2img - Extract Images from Duke Nukem 3D Art Files

A modern C++ tool to convert Duke Nukem 3D art files into PNG or TGA images with zero-copy processing and modern C++17 features.

## Features
- **Modern C++17 Codebase**: Utilizes std::filesystem, constexpr optimizations, and modern API design
- **Zero-Copy Processing**: ArtView and ImageView structures for efficient memory access
- **Format Support**: PNG (with alpha transparency) and TGA output formats
- **Magenta Transparency**: Automatic handling of transparency for game assets
- **Parallel Processing**: Multi-threaded extraction with ordered output
- **Cross-Platform**: Windows, Linux, and macOS support via CMake
- **Comprehensive Testing**: Modern doctest-based test framework

## Quick Start
```bash
# Clone the repository
git clone https://github.com/raulcorreia7/art2img.git
cd art2img

# Build using the convenience script (recommended)
./build.sh

# Or use the wrapper Makefile
make

# Run a quick extraction
./build/bin/art2img -f png -o output tests/assets/TILES000.ART
```

## Example
- Basic usage:
  ```bash
  # Extract a single file
  ./bin/art2img TILES000.ART
  ```

- Directory with merged files:
  ```bash
  # Extract a directory of files
  ./bin/art2img -m -o /output /path
  ```

## Output Formats
- Output formats in PNG and TGA
  - PNG output with RGBA support
  - PNG output with proper alpha channel
  - Automatic transparency for magenta pixels
  - Automatic output with premultiplied alpha

## Command Line Options
- `--version` or `-v`  show version information
- `--format` or `-f` to specify the output format
- `--output` or `-o` to specify the output directory
- `-p` for palette file
- `-n` for no animation
- `-m` for merging animation data
- `-t` for specifying threads
- `-q` for quiet output

## Building

### Method 1: Convenience Script (Recommended)
```bash
./build.sh
```

### Method 2: Makefile Wrapper
The Makefile is now a wrapper around CMake, providing familiar targets:
```bash
make help          # Show all available targets
make all           # Build all targets (default)
make test          # Build and run tests
make clean         # Clean build artifacts
make install       # Install to system
make debug         # Build with debug configuration
make release       # Build with release configuration
```

### Method 3: Direct CMake
```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . --parallel $(nproc)

# Run tests
ctest --output-on-failure

# Install (optional)
cmake --build . --target install  # or 'sudo make install'
```

### CMake Options
- `-DBUILD_CLI=ON/OFF` - Build the command line tool (default: ON)
- `-DBUILD_TESTS=ON/OFF` - Build tests (default: ON)
- `-DBUILD_SHARED_LIBS=ON/OFF` - Build shared/static library (default: ON)
- `-DCMAKE_BUILD_TYPE=Release/Debug` - Build type

### Cross-Platform Builds
```bash
# Linux (default)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Windows cross-compilation (requires mingw-w64)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake
```

## Library Usage

The project provides a modern C++ library (`libart2img_extractor`) with zero-copy processing capabilities:

### Modern API Usage
```cpp
#include <extractor_api.hpp>
#include <filesystem>
#include <iostream>

int main() {
    art2img::ExtractorAPI extractor;

    // Load files using modern std::filesystem::path
    extractor.load_palette_file("palette.dat");
    extractor.load_art_file("tiles.art");

    // Extract a single tile
    auto result = extractor.extract_tile(0);

    if (result.success) {
        std::cout << "Extracted tile " << result.tile_index
                  << ": " << result.width << "x" << result.height
                  << " (" << result.format << ")" << std::endl;

        // Access animation data
        std::cout << "Animation: " << result.anim_frames << " frames, "
                  << "type " << result.anim_type << std::endl;
    }

    // Get zero-copy view for parallel processing
    auto art_view = extractor.get_art_view();

    // On-demand ImageView access
    for (size_t i = 0; i < art_view.image_count(); ++i) {
        art2img::ImageView image{&art_view, static_cast<uint32_t>(i)};

        // Extract directly to memory or file
        if (!image.pixel_data()) continue;  // Skip empty tiles

        auto png_data = image.extract_to_png();
        // Process png_data...
    }

    return 0;
}
```

### Key API Features
- **Zero-Copy ArtView**: Direct memory access to ART file data
- **ImageView**: On-demand tile rendering without copying
- **Modern Path Handling**: Full std::filesystem::path support
- **Compile-time Optimizations**: constexpr functions for metadata access
- **Animation Support**: Complete extraction of animation metadata
- **Memory and File APIs**: Load from files or memory buffers

### Linking with CMake
```cmake
find_package(PkgConfig REQUIRED)
pkg_check_modules(ART2IMG REQUIRED art2img-extractor)

add_executable(my_app main.cpp)
target_link_libraries(my_app ${ART2IMG_LDFLAGS})
target_include_directories(my_app PRIVATE ${ART2IMG_INCLUDE_DIRS})
```

### Manual Linking
```bash
# Compile with
c++ -o my_app main.cpp -lart2img_extractor -I/usr/local/include

# Or with pkg-config
c++ -o my_app main.cpp $(pkg-config --cflags --libs art2img-extractor)
```

## Requirements
- **Build requirements**:
  - C++17 compiler (GCC 7+, Clang 6+, MSVC 2019+)
  - CMake 3.14+
  - pthread library
  - std::filesystem support (GCC 9+ with libstdc++fs, or Clang 9+)
- **Runtime requirements**:
  - No external dependencies for the CLI tool
  - Library requires only standard C++17 runtime
  - stb_image_write library (bundled, no external dependency required)
  - doctest framework (bundled for tests only)

## Architecture
The codebase has been modernized with:
- **Zero-Copy Processing**: ArtView and ImageView structures for efficient memory access
- **Modern C++17**: std::filesystem::path, constexpr optimizations, RAII
- **Test Framework**: Migrated to doctest v2.4.11 for modern C++ testing
- **Build System**: CMake-first approach with Makefile wrapper for compatibility
- **API Design**: Clean separation between library and CLI components

## Recent Modernization
- ✅ Removed legacy ArtExtractor class in favor of ArtView-based API
- ✅ Implemented zero-copy memory processing for better performance
- ✅ Added std::filesystem::path support throughout the codebase
- ✅ Added constexpr optimizations for metadata access functions
- ✅ Migrated to doctest testing framework
- ✅ Converted Makefile to CMake wrapper for simplified builds

## Development
- **Testing**: Run `make test` or `ctest --output-on-failure`
- **Debug Builds**: Use `make debug` for debugging symbols
- **Code Style**: Modern C++17 conventions with const-correctness
- **Memory Safety**: Zero-copy design minimizes allocations during processing