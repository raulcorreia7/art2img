# art2img

**Convert Build Engine ART files to modern image formats**

A minimal, modern C++ library and CLI tool for converting Duke Nukem 3D and other Build Engine ART files to PNG, TGA, or BMP formats.

## Features

- **Clean C++17 API**: Non-throwing, Result-based error handling
- **GRP Archive Support**: Extract ART files from Duke3D GRP archives
- **Full Rendering Options**: Shade tables, lookup tables, transparency, premultiply, matte hygiene
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
```

## Library Usage

```cpp
#include <art2img.hpp>

// Load ART file
auto art = art2img::art_file::load("TILES000.ART");
if (!art) { /* handle error */ }

// Load palette
auto pal = art2img::palette::load("PALETTE.DAT");
if (!pal) { /* handle error */ }

// Extract all tiles
auto images = art2img::extract_all(art.value(), pal.value());
if (!images) { /* handle error */ }

// Save as PNG
for (size_t i = 0; i < images.value().size(); ++i) {
    auto png = art2img::encode(images.value()[i], art2img::format::png);
    art2img::write_file("tile_" + std::to_string(i) + ".png", png.value());
}
```

## API Overview

| Class/Function | Description |
|----------------|-------------|
| `grp_file` | Duke3D GRP archive handling |
| `art_file` | Build Engine ART file parser |
| `palette` | 768-byte palette with shade table support |
| `render()` | Convert tile to RGBA image |
| `encode()` | Encode to PNG/TGA/BMP |
| `extract_all()` | Extract all tiles from ART |
| `convert_art()` | Batch convert and save |

## Error Handling

All operations return `Result<T>` which is either a value or an error:

```cpp
auto result = art2img::art_file::load("file.art");
if (!result) {
    std::cerr << result.message() << "\n";
    // Error codes: io_error, invalid_format, corrupted_data, not_found, etc.
}
```

## License

GPL v2 - See LICENSE file
