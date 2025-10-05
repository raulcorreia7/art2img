<div align="center">
  <h1>art2img</h1>
  <p><strong>Convert Duke Nukem 3D ART files to modern image formats</strong></p>
  <p>
    <a href="https://github.com/raulcorreia7/art2img/actions"><img src="https://github.com/raulcorreia7/art2img/workflows/CI/badge.svg" alt="CI Status"></a>
    <a href="LICENSE"><img src="https://img.shields.io/github/license/raulcorreia7/art2img" alt="License"></a>
    <a href="https://github.com/raulcorreia7/art2img/releases"><img src="https://img.shields.io/github/v/release/raulcorreia7/art2img" alt="Release"></a>
  </p>
</div>

## What is art2img?

A modern C++20 tool that converts Duke Nukem 3D ART files to PNG, TGA, or BMP with zero-copy processing and high performance.

Perfect for:
- **Game modders** extracting assets
- **Developers** building tools
- **Archivists** converting legacy formats

## Features

- **Ultra-fast** - Zero-copy processing with modern C++20
- **Multiple formats** - PNG, TGA, BMP with alpha transparency
- **Cross-platform** - Linux, Windows, macOS
- **Library & CLI** - Use as library or standalone tool
- **Advanced options** - Transparency, animation, custom palettes

## Quick Start

```bash
# Build
make all

# Convert files
./build/linux-release/bin/art2img tiles.art
./build/linux-release/bin/art2img tiles.art -f png -o output/
./build/linux-release/bin/art2img art/ -o images/
```

## Usage

```bash
# Basic conversion
./build/linux-release/bin/art2img tiles.art

# Specify format and output
./build/linux-release/bin/art2img tiles.art -f tga -o ./output

# Custom palette
./build/linux-release/bin/art2img tiles.art -p custom.pal

# Directory processing
./build/linux-release/bin/art2img art/ -m -o game/

# Disable transparency
./build/linux-release/bin/art2img tiles.art --no-fix-transparency
```

## Installation

```bash
# Clone
git clone https://github.com/raulcorreia7/art2img.git
cd art2img

# Build
make all

# Install system-wide
sudo make install
```

## Keeping Your Fork Up to Date

Before attempting to pull updates, make sure your local clone has a remote configured for the upstream repository:

```bash
git remote add origin git@github.com:your-org/art2img.git
git pull origin master
```

Replace the remote URL with the appropriate upstream for your fork or workspace before syncing.

## Building

**Requirements:**
- C++20 compiler (GCC 10+, Clang 10+, MSVC 2019+)
- CMake 3.14+
- pthread (Linux)

**Build options:**
```bash
make all          # Build for Linux (default)
make build        # Build for Linux
make windows      # Cross-compile for Windows x64
make windows-x86  # Cross-compile for Windows x86
make linux-release       # Release build + tests for Linux
make windows-release     # Release build for Windows x64
make windows-x86-release # Release build for Windows x86
make test         # Run tests on Linux
make test-windows # Test Windows x64 build (requires Wine)
make test-windows-x86 # Test Windows x86 build (requires Wine)
make clean        # Clean build directory
make doctor       # Check host dependencies
```

## Verification

Run the test suite locally before pushing changes to ensure everything stays green:

```bash
make test
```

For a release-style verification that configures a fresh build directory and executes the same tests, use:

```bash
make linux-release
```

Both targets rebuild the project as needed so you can rely on them to catch regressions early.

## Documentation

- [Building Instructions](BUILDING.md) - Detailed build guide
- [Library API](https://github.com/raulcorreia7/art2img) - API documentation
- [License](LICENSE) - GPL v2 license

## Credits

Based on original work by Mathieu Olivier and Kenneth Silverman.
Modern C++20 implementation by [Raul Correia](https://github.com/raulcorreia7).

## License

[GPL v2](LICENSE) - Free and open-source software.