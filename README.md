# art2img

**Convert Build Engine ART files to modern image formats**

A minimal, modern C++ library and CLI tool for converting Duke Nukem 3D and other Build Engine ART files to PNG, TGA, or BMP formats.

## Features

- **Clean C++17 API**: Non-throwing, Result-based error handling
- **Thread-Safe**: Stateless library design, consumer handles threading
- **GRP Archive Support**: Extract ART files from Duke3D GRP archives
- **Full Rendering Options**: Shade tables, lookup tables, transparency, premultiply, matte hygiene
- **Parallel Conversion**: Multi-threaded tile conversion via `-j` / `--jobs`
- **Zero Dependencies**: Only requires stb_image_write (vendored)
- **Simple Build**: Single CMakeLists.txt, no external package managers

## Quick Start

```bash
# Build
mkdir build && cd build
cmake .. && make -j

# Run tests
./art2img-test

# Convert ART file
./art2img TILES000.ART PALETTE.DAT -o output/

# Convert from GRP
./art2img --grp DUKE3D.GRP -o output/ --verbose

# Parallel conversion (8 threads)
./art2img --grp DUKE3D.GRP -o output/ -j 8

# Quiet mode
./art2img --grp DUKE3D.GRP -o output/ -q
```

## CLI Options

```
Usage: art2img [options] [input] [palette]

Modes:
  --list                List GRP contents and exit
  -h, --help           Show this help
  --version            Show version

Input Options:
  --grp <file>         Load ART from GRP archive
  --art <name>         ART filename in GRP (default: auto-detect)
  -p, --palette <file> Palette file (default: auto-detect from GRP)

Output Options:
  -o, --output <dir>   Output directory (default: current)
  -f, --format <fmt>   Output format: png, tga, bmp (default: png)

Rendering Options:
  --shade <n>          Apply shade table index (0-31)
  --lookup             Enable lookup table remapping
  --no-transparency    Disable transparency fix (index 255 opaque)
  --premultiply        Enable alpha premultiplication
  --matte              Enable matte hygiene

Performance:
  -j, --jobs <n>      Number of parallel jobs (0=auto)
  --no-parallel        Disable parallel processing

Other:
  -v, --verbose        Verbose output
  -q, --quiet          Suppress non-error output
```

## Library Usage

```cpp
#include <art2img.hpp>

// Load ART file
auto art = art2img::ArtFile::load("TILES000.ART");
if (!art) { /* handle error */ }

// Load palette
auto pal = art2img::Palette::load("PALETTE.DAT");
if (!pal) { /* handle error */ }

// Extract all tiles
auto images = art2img::extract_all(art.value(), pal.value());
if (!images) { /* handle error */ }

// Save as PNG
for (size_t i = 0; i < images.value().size(); ++i) {
    auto png = art2img::encode(images.value()[i], art2img::Format::png);
    art2img::write_file("tile_" + std::to_string(i) + ".png", png.value());
}
```

## API Overview

| Class/Function | Description |
|----------------|-------------|
| `GrpFile` | Duke3D GRP archive handling |
| `ArtFile` | Build Engine ART file parser |
| `Palette` | 768-byte palette with shade table support |
| `render()` | Convert tile to RGBA image |
| `encode()` | Encode to PNG/TGA/BMP |
| `extract_all()` | Extract all tiles from ART |
| `convert_art()` | Batch convert and save |

## Error Handling

All operations return `Result<T>` which is either a value or an error:

```cpp
auto result = art2img::ArtFile::load("file.art");
if (!result) {
    std::cerr << result.message() << "\n";
    // Error codes: io_error, invalid_format, corrupted_data, not_found, etc.
}
```

## License

GPL v2 - See LICENSE file
