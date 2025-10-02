# art2img

ART file extractor for Duke Nukem 3D assets. Extracts ART files to PNG or TGA images with proper alpha channel support. Optimized for containerized pipeline execution.

## Overview

art2img is a C++ tool for extracting images from Duke Nukem 3D ART files. It provides multi-threaded extraction with support for modern image formats and professional alpha channel handling.

## Features

- Multi-threaded ART file extraction
- PNG and TGA output formats with RGBA support
- Proper alpha channel handling with magenta keying
- Animation data extraction (animdata.ini)
- Duke3D palette support with 6-bit to 8-bit conversion
- Container-optimized Linux build

## Quick Start

### 📥 Download Pre-built Binaries

[![Latest Release](https://img.shields.io/github/release/raulcorreia7/art2img.svg)](https://github.com/raulcorreia7/art2img/releases/latest)

- **Linux x86_64**: `art2img-linux-x86_64-*.tar.gz`
- **Windows x86_64**: `art2img-windows-x86_64-*.zip`

### Docker Pipeline (Recommended)

```bash
# Build and test in container
docker build -t art2img .

# Extract ART files
docker run --rm -v $(pwd)/input:/input -v $(pwd)/output:/output art2img /input/tiles.art -o /output

# Process directory of ART files
docker run --rm -v $(pwd)/assets:/assets -v $(pwd)/output:/output art2img -m /assets -o /output
```

### Local Build

```bash
# Build Linux binaries
make

# Build all platforms (requires cross-compilers)
make build-all

# Run tests
make test

# Clean build artifacts
make clean

# Extract ART files
./bin/art2img tests/assets/TILES000.art -o ./output
```

### Cross-Platform Building

```bash
# Build Windows binaries (requires MinGW)
make windows

# Verify all binaries
make verify
```

## Command Line Options

```
Usage: art2img [OPTIONS] <ART_FILE|ART_DIRECTORY>

Options:
  -o, --output DIR     Output directory (default: current)
  -t, --threads N      Number of threads (default: CPU cores)
  -p, --palette FILE   Palette file path (default: auto-detect)
  -f, --format FMT     Output format: tga or png (default: png)
  -q, --quiet          Suppress verbose output
  -n, --no-anim        Don't generate animdata.ini
  -m, --merge-anim     Merge animation data into single file
  -h, --help           Show this help message
  -v, --version        Show version information
```

## Alpha Channel Support

PNG output includes proper alpha channel handling:
- Magenta pixels (r8≥250, b8≥250, g8≤5) are treated as transparent
- Premultiplied alpha for proper upscaling
- Optional matte hygiene for clean edges

## Project Structure

```
src/           - C++ source files
include/       - Header files
bin/           - Compiled binaries
vendor/        - Third-party dependencies
tests/         - Test assets and scripts
```

## Requirements

### For Building
- C++17 compatible compiler (g++)
- POSIX threads (pthreads)
- Docker (for containerized execution)

### Cross-Compilation (Optional)
- **Windows**: `g++-mingw-w64-x86-64`

### For Running
- **Linux**: No additional dependencies (static binaries)
- **Windows**: No additional dependencies (static binaries)
- **Docker**: Docker runtime (for containerized execution)

## Supported Platforms

| Platform | Architecture | Build Status |
|----------|-------------|--------------|
| Linux | x86_64 | ✅ |
| Windows | x86_64 | ✅ |

All binaries are statically linked and require no additional runtime dependencies.