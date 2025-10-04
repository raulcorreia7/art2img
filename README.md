# art2img - Extract Images from Duke Nukem 3D Art Files

A modern C++ tool to convert Duke Nukem 3D art files into PNG or TGA images with zero-copy processing and modern C++20 features.

## Features
- **Modern C++20 Library**: Core functionality available as a reusable library
- **Command Line Interface**: Standalone CLI tool for direct usage
- **Zero-Copy Processing**: ArtView and ImageView structures for efficient memory access
- **Format Support**: PNG (with alpha transparency) and TGA output formats
- **Magenta Transparency**: Automatic handling of transparency for game assets
- **Cross-Platform**: Windows, Linux, and macOS support via CMake
- **Comprehensive Testing**: Modern doctest-based test framework

## Quick Start
```bash
# Clone the repository
git clone https://github.com/raulcorreia7/art2img.git
cd art2img

# Build with Makefile (recommended)
make all

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

### Makefile (recommended)
The project includes a Makefile that simplifies building, testing, and installation. It wraps CMake and caches dependencies for faster repeated builds.

```bash
make all          # Build everything and run tests
make build        # Configure and build in Release mode
make test         # Build and run tests
make debug        # Build in Debug mode
make clean        # Remove build directory
make install      # Install to /usr/local (requires build)
make cache-clean  # Clear the dependency cache (_fetch)
make asan-build   # Build with AddressSanitizer
make leak-build   # Build with LeakSanitizer
make test-asan    # Run tests with AddressSanitizer
make test-leak    # Run tests with LeakSanitizer

# Customizable variables
make build JOBS=4 CMAKE_BUILD_TYPE=Release BUILD_DIR=mybuild
```

Dependencies are cached in `~/.cache/art2img/fetchcontent/` using CMake's FetchContent mechanism. First configure downloads them; subsequent builds reuse the cache for speed (especially in CI).

For CMake options like `-DBUILD_CLI=OFF`, pass them via `CMAKE_ARGS="-DBUILD_CLI=OFF make build"`.

### CMake (manual)
If you prefer direct CMake usage:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release $(make -s print-cmake-args)  # Optional: use dep flags from Makefile
cmake --build . --parallel $(nproc)

# Run tests
ctest --output-on-failure

# Install (optional)
cmake --build . --target install
```

### CMake Options
- `-DBUILD_CLI=ON/OFF` – build the command line tool (default ON)
- `-DBUILD_TESTS=ON/OFF` – build doctest suite (default ON)
- `-DBUILD_SHARED_LIBS=ON/OFF` – choose shared vs static library (default ON)
- `-DCMAKE_BUILD_TYPE=Release/Debug` – build type

Pass options to Makefile via `CMAKE_ARGS="-DBUILD_TESTS=OFF" make build`.

### Cross-Platform Builds
The Makefile supports cross-compilation; set `CMAKE_TOOLCHAIN_FILE` in `CMAKE_ARGS`.

```bash
# Linux (default)
make build

# Windows cross-compilation (requires mingw-w64)
make build CMAKE_ARGS="-DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake"
```


### CMake Options
- `-DBUILD_CLI=ON/OFF` – build the command line tool (default ON)
- `-DBUILD_TESTS=ON/OFF` – build doctest suite (default ON)
- `-DBUILD_SHARED_LIBS=ON/OFF` – choose shared vs static library (default ON)
- `-DCMAKE_BUILD_TYPE=Release/Debug` – build type

### Cross-Platform Builds
```bash
# Linux (default)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Windows cross-compilation (requires mingw-w64)
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake
```

## Library Usage

The project provides a modern C++ library (`libart2img_extractor`) with zero-copy processing capabilities. The library is separate from the CLI tool, allowing external projects to consume only the core functionality:

### Library Integration

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
  - C++20 compiler (GCC 10+, Clang 10+, MSVC 2019+)
  - CMake 3.14+
  - pthread library
  - std::filesystem support (GCC 9+ with libstdc++fs, or Clang 9+)
- **Runtime requirements**:
  - No external dependencies for the CLI tool
  - Library requires only standard C++20 runtime
- stb_image_write header (fetched automatically via CMake)
- doctest framework (fetched automatically for the test suite)

## Architecture
The codebase has been modernized with:
- **Library-CLI Separation**: Core library (`libart2img_extractor`) separate from CLI tool (`art2img`)
- **Zero-Copy Processing**: ArtView and ImageView structures for efficient memory access
- **Modern C++20**: std::filesystem::path, constexpr optimizations, RAII, and more
- **Test Framework**: Migrated to doctest v2.4.11 for modern C++ testing
- **Build System**: CMake-first approach with FetchContent-managed dependencies
- **API Design**: Clean separation between library and CLI components

## Recent Modernization
- ✅ Removed legacy ArtExtractor class in favor of ArtView-based API
- ✅ Implemented zero-copy memory processing for better performance
- ✅ Added std::filesystem::path support throughout the codebase
- ✅ Added constexpr optimizations for metadata access functions
- ✅ Migrated to doctest testing framework with FetchContent-based setup

## Development
- **Testing**: `ctest --test-dir build --output-on-failure` - Run all tests (doctest and standalone)
- **Debug builds**: `cmake --build build --config Debug`
- **Style**: Modern C++20 conventions with const-correctness and RAII
- **Code Formatting**: Google C++ style enforced via clang-format
- **Memory Safety**: Zero-copy design minimizes allocations during processing

## Testing
This project includes a comprehensive doctest-based test suite:

- **All tests**: `make test` or `ctest --output-on-failure` - Run all unit tests
- **Doctest tests**: `./build/bin/art2img_tests` - Run complete unit test suite
- The test suite includes tests for:
  - Art file parsing and metadata extraction
  - Palette handling and validation
  - Image view and extraction functionality
  - API integration and memory regression
  - Blood palette validation
  - Comprehensive library API functionality

## Code Formatting
This project uses clang-format with Google C++ style for consistent code formatting. You can format your code using:

- Using the Makefile (recommended):
  ```bash
  make format                    # Format all source files
  make format-check             # Check if all files are properly formatted
  ```

- Using CMake directly:
  ```bash
  mkdir build && cd build
  cmake ..
  cmake --build . --target clang-format    # Format all files
  ```

- Using the provided script:
  ```bash
  ./format_code.sh             # Format all source files
  ```

## Memory Safety Testing

The project includes support for memory sanitizers to detect memory leaks and other memory-related issues:

### AddressSanitizer (ASAN)
AddressSanitizer is a fast memory error detector that can detect:
- Buffer overflows
- Use after free
- Memory leaks
- Double free errors

```bash
# Build with AddressSanitizer
make asan-build

# Run tests with AddressSanitizer
make test-asan
```

### LeakSanitizer
LeakSanitizer is a memory leak detector that can be used standalone:

```bash
# Build with LeakSanitizer
make leak-build

# Run tests with LeakSanitizer
make test-leak
```

### Interpreting Results
When running tests with sanitizers, any memory issues will be reported with detailed information including:
- Stack traces for memory errors
- Memory leak sizes and locations
- Suggested fixes

Note: Sanitizer builds are only supported on Linux and macOS with GCC or Clang compilers.
