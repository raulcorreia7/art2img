# art2img Usage

## Command Line

The CLI converts Build Engine ART tiles into standard image files using a
palette-driven, memory-first workflow.

```bash
# Convert tiles from an ART file using the palette alongside it
art2img_cli --input TILES.ART --palette PALETTE.DAT --output out/

# Generate TGA output with shading and matte hygiene enabled
art2img_cli --input TILES.ART --palette PALETTE.DAT \
            --format tga --shade 4 --matte

# Write 32-bit premultiplied PNG images without palette remapping
art2img_cli --input TILES.ART --palette PALETTE.DAT \
            --premultiply --no-lookup
```

### Options

| Option | Description |
| ------ | ----------- |
| `-i, --input <path>` | Path to the ART file to convert (required). |
| `-p, --palette <path>` | Palette file providing RGB, shade, and lookup data (required). |
| `-o, --output <dir>` | Directory where encoded images are written (default: current directory). |
| `-f, --format <png|tga|bmp>` | Output image format (default: `png`). |
| `--shade <value>` | Shade table index to apply during conversion (0-255). |
| `--no-lookup` | Disable palette lookup remapping. |
| `--no-transparency` | Skip transparency cleanup for palette index 0. |
| `--premultiply` | Premultiply the alpha channel in the resulting RGBA pixels. |
| `--matte` | Apply matte hygiene to soften semi-transparent edges. |

The CLI reads the palette and ART assets fully into memory, converts every tile
with the requested options, and writes encoded images directly to the target
folder.

## Library API

The public API is organised around the `core`, `adapters`, and `extras`
namespaces. Typical programs load binary resources using adapters, transform
tiles in memory with the core module, and optionally orchestrate batches through
`art2img::extras` helpers.

```cpp
#include <art2img/adapters/io.hpp>
#include <art2img/core/art.hpp>
#include <art2img/core/convert.hpp>
#include <art2img/core/encode.hpp>
#include <art2img/core/palette.hpp>

using namespace art2img;

// Load assets from disk
auto art_bytes = adapters::read_binary_file("TILES.ART");
auto palette_bytes = adapters::read_binary_file("PALETTE.DAT");

auto art = core::load_art(std::span<const std::byte>(art_bytes->data(),
                                                     art_bytes->size()));
auto palette = core::load_palette(std::span<const std::byte>(
    palette_bytes->data(), palette_bytes->size()));

const auto palette_view = core::view_palette(*palette);
const std::size_t count = core::tile_count(*art);

for (std::size_t index = 0; index < count; ++index) {
  auto tile = core::get_tile(*art, index);
  if (!tile) continue;

  auto rgba = core::palette_to_rgba(
      *tile, palette_view, core::ConversionOptions{.apply_lookup = true});
  core::postprocess_rgba(*rgba, core::PostprocessOptions{});

  auto encoded = core::encode_image(core::make_view(*rgba), core::ImageFormat::png);
  adapters::write_file(std::format("tile_{:04}.png", index),
                       std::span<const std::byte>(encoded->bytes.data(),
                                                  encoded->bytes.size()));
}
```

For bulk operations, the `art2img::extras::BatchRequest` helper performs tile
conversion and encoding in a single call while still keeping the workflow
memory-first.

## Requirements

* Provide a valid Build Engine palette (typically `PALETTE.DAT`).
* Use a modern C++23 compiler. The library links against `stb_image_write` for
  PNG/TGA/BMP output.

Include `<art2img/api.hpp>` for the entire public surface or pull in individual
headers from `include/art2img/core`, `include/art2img/adapters`, and
`include/art2img/extras` as needed.
