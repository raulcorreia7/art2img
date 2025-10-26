# art2img

**Convert Build Engine ART files to modern image formats**

A command-line utility and C++ library for converting Duke Nukem 3D and other Build Engine ART files to PNG, TGA, or BMP formats. Designed for game modders who need to extract and modify game assets.

## Quick Start

### Option 1: Download Pre-built Binaries (Recommended)
Pre-built binaries for Windows, Linux, and macOS are available on the [Releases page](https://github.com/raulcorreia7/art2img/releases).

### Option 2: Build from Source

**Requirements:** CMake and a C++ compiler

```bash
# Clone and build
make build

# The executable will be at: build/linux-x64/cli/art2img_cli
# Or for Windows: build/windows-x64/cli/art2img_cli.exe
```

## Basic Usage

```bash
# Convert a single ART file to PNG using an explicit palette
art2img_cli --input TILES.ART --palette PALETTE.DAT

# Write TGA images to a custom directory with matte hygiene enabled
art2img_cli --input TILES.ART --palette PALETTE.DAT --format tga --matte --output output/

# Apply shade table index 4 without lookup remapping
art2img_cli --input TILES.ART --palette PALETTE.DAT --shade 4 --no-lookup
```

## Key Features

- **Multiple Formats**: Encode tiles as PNG, TGA, or BMP images.
- **Palette-Aware Pipeline**: Memory-first conversion with lookup tables and shade support.
- **Post-Processing Controls**: Configure transparency cleanup, alpha premultiplication, and matte hygiene.
- **Reusable Modules**: Compose loaders, converters, and encoders from the `core`, `adapters`, and `extras` namespaces.
- **Cross-Platform**: Works on Windows, Linux, and macOS with a modern C++23 toolchain.
- **Enhanced Error Handling**: Unified `core::Error` reports contextual diagnostics across modules.

## Command Line Options

```
art2img_cli --input TILES.ART --palette PALETTE.DAT [options]

-i, --input PATH        ART file to convert (required)
-p, --palette PATH      Palette file to use (required)
-o, --output DIR        Output directory (default: current directory)
-f, --format FORMAT     Output format: png, tga, bmp (default: png)
    --shade INT         Apply shade table index (0-255)
    --no-lookup         Disable lookup table remapping
    --no-transparency   Skip transparency cleanup
    --premultiply       Premultiply the alpha channel
    --matte             Apply matte hygiene to soften edges
```

## Library Usage

The art2img library provides a clean, modern C++ API for integrating ART file conversion into your own applications:

```cpp
#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>
#include <art2img/core/palette.hpp>

auto art_bytes = art2img::adapters::read_binary_file("TILES.ART");
auto palette_bytes = art2img::adapters::read_binary_file("PALETTE.DAT");

auto archive = art2img::core::load_art(std::span<const std::byte>(
    art_bytes->data(), art_bytes->size()));
auto palette = art2img::core::load_palette(std::span<const std::byte>(
    palette_bytes->data(), palette_bytes->size()));

const auto palette_view = art2img::core::view_palette(*palette);
for (std::size_t index = 0; index < art2img::core::tile_count(*archive); ++index) {
    auto tile = art2img::core::get_tile(*archive, index);
    if (!tile) continue;

    auto rgba = art2img::core::palette_to_rgba(*tile, palette_view);
    art2img::core::postprocess_rgba(*rgba);

    auto encoded = art2img::core::encode_image(
        art2img::core::make_view(*rgba), art2img::core::ImageFormat::png);
    art2img::adapters::write_file(
        std::format("tile_{:04}.png", index),
        std::span<const std::byte>(encoded->bytes.data(), encoded->bytes.size()));
}
```

## Troubleshooting

**Transparency issues:** Transparency cleanup is enabled by default. Use `--no-transparency` to keep raw palette data.

**Shading:** Provide `--shade <index>` to apply a specific shade table during conversion.

**Custom palette:** Always supply the palette file that matches the ART resources with `--palette`.

## License

[GPL v2](LICENSE) - Free and open-source software.

## Credits

Based on original work by Mathieu Olivier and Kenneth Silverman.
Modern implementation by [Raúl Correia](https://github.com/raulcorreia7).