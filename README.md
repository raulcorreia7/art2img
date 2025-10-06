<div align="center">
  <h1>art2img v0.1.0</h1>
  <p><strong>Convert Duke Nukem 3D ART files to modern image formats</strong></p>
</div>

A modern C++20 tool that converts Duke Nukem 3D ART files to PNG, TGA, or BMP with zero-copy processing and high performance.
Perfect for game modders extracting assets.

## Features

- **Ultra-fast** - Zero-copy processing with modern C++20
- **Multiple formats** - PNG, TGA, BMP with alpha transparency
- **Cross-platform** - Linux, Windows, macOS
- **Advanced options** - Transparency, animation, custom palettes

## Quick Start

```bash
# Build
make all

# Convert files
./build/linux-x64-release/bin/art2img tiles.art
./build/linux-x64-release/bin/art2img tiles.art -f png -o output/
./build/linux-x64-release/bin/art2img art/ -o images/
```

## Usage

```bash
# Basic conversion
./build/linux-x64-release/bin/art2img tiles.art

# Specify format and output
./build/linux-x64-release/bin/art2img tiles.art -f tga -o ./output

# Custom palette
./build/linux-x64-release/bin/art2img tiles.art -p custom.pal

# Directory processing
./build/linux-x64-release/bin/art2img art/ -m -o game/

# Disable transparency
./build/linux-x64-release/bin/art2img tiles.art --no-fix-transparency
```

## Building

**Requirements:**
- C++20 compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.14+

**Build options:**
```bash
make all          # Build for Linux (default)
make build        # Build for Linux x64
make mingw-windows      # Cross-compile for Windows x64 using MinGW
make mingw-windows-x86  # Cross-compile for Windows x86 using MinGW
make linux-x64-release       # Release build + tests for Linux x64
make mingw-windows-x64-release     # Release build for Windows x64
make mingw-windows-x86-release # Release build for Windows x86
make test         # Run tests on Linux
make clean        # Clean build directory
```

## Credits

Based on original work by Mathieu Olivier and Kenneth Silverman.
Modern C++20 implementation by [Raul Correia](https://github.com/raulcorreia7).

## License

[GPL v2](LICENSE) - Free and open-source software.