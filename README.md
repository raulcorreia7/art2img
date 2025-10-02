# art2img - Extract Images from Duke Nukem 3D Art Files

A simple tool to convert Duke Nukem 3D art files into modern PNG or TGA images.

## What's New
- **Version**: 1.0.0
- **Version support**: Added the ability to check the version with `--version` or `-v`
- **Transparency**: Added the ability to automatically handle magenta transparency

## Quick Start
- Download the latest release from the [GitHub releases](https://github.com/raulcorreia7/art2img/releases)
- Extract the art files with the art2img command
- Run the command to check the version with the command `--version`

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

## Building with CMake

### Quick Start
```bash
# Clone the repository
git clone https://github.com/raulcorreia7/art2img.git
cd art2img

# Build using the provided script (recommended)
./build.sh

# Or build manually
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
- `-DBUILD_DIAGNOSTIC=ON/OFF` - Build diagnostic tool (default: OFF)
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

The project provides a C++ library (`libart2img_extractor`) that can be used in other projects:

### Using the Library
```cpp
#include <extractor_api.hpp>
#include <iostream>

int main() {
    art2img::ExtractorAPI extractor;

    // Load palette and art file
    extractor.load_palette_file("palette.dat");
    extractor.load_art_file("tiles.art");

    // Extract a single tile
    auto result = extractor.extract_tile(0);

    if (result.success) {
        // Use the image data...
        std::cout << "Extracted tile 0: " << result.width << "x" << result.height << std::endl;
    }

    return 0;
}
```

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
  - C++17 compiler (GCC, Clang, MSVC)
  - CMake 3.14+
  - pthread library
- **Runtime requirements**:
  - No external dependencies for the CLI
  - Library requires standard C++ runtime