# art2img

Convert Build Engine ART files to PNG, TGA, or BMP.

## Install

```bash
# Build
mkdir build && cd build
cmake .. && make

# Or use Makefile
make build
```

## Usage

```bash
# Convert from GRP archive
./art2img --grp DUKE3D.GRP -o output/

# Convert standalone ART
./art2img TILES000.ART PALETTE.DAT -o output/

# List GRP contents
./art2img --grp DUKE3D.GRP --list

# Parallel (8 threads)
./art2img --grp DUKE3D.GRP -o output/ -j 8
```

## Options

| Option | Description |
|--------|-------------|
| `--grp <file>` | Load from GRP archive |
| `-o <dir>` | Output directory |
| `-f <fmt>` | Format: png, tga, bmp (default: png) |
| `-j <n>` | Parallel jobs (0=auto) |
| `--shade <n>` | Shade table (0-31) |
| `--no-transparency` | Disable transparency fix |
| `--premultiply` | Alpha premultiplication |
| `-q` | Quiet mode |
| `-v` | Verbose |

## Library

```cpp
#include <art2img.hpp>

auto art = art2img::ArtFile::load("TILES000.ART");
auto pal = art2img::Palette::load("PALETTE.DAT");
auto images = art2img::extract_all(*art, *pal);
```

## License

GPL v2
