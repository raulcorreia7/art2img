# Iteration Plan — Memory-First Core Promotion

This plan tracks the work required to ship the simplified, memory-first API and
ensure the CLI and helpers rely exclusively on the new modules.

## Objectives

1. Remove legacy headers and sources so only `art2img::core`, `adapters`, and
   `extras` remain publicly visible.
2. Align the CLI and documentation with the new workflow (memory-first loads,
   palette-to-RGBA conversion, post-processing, encoding, and IO adapters).
3. Restore focused test coverage for the promoted modules.
4. Produce updated iteration notes and developer guidance.

## Module Inventory

| Namespace | Responsibility | Key Types |
|-----------|----------------|-----------|
| `core`    | ART parsing, palette loading, conversion, encoding, error handling | `ArtArchive`, `Palette`, `TileView`, `ConversionOptions`, `PostprocessOptions`, `RgbaImage`, `EncodedImage`, `Error` |
| `adapters` | File IO, GRP parsing, manifest formatting | `read_binary_file`, `write_file`, `load_grp`, `format_animation_ini/json` |
| `extras` | Batch helpers orchestrating core operations | `BatchRequest`, `BatchResult` |

## Updated API Surface

```cpp
// art loading
std::expected<core::ArtArchive, core::Error>
core::load_art(std::span<const std::byte> blob);

// palette loading
std::expected<core::Palette, core::Error>
core::load_palette(std::span<const std::byte> blob);

// tile access
std::size_t core::tile_count(const core::ArtArchive&);
std::optional<core::TileView> core::get_tile(const core::ArtArchive&, std::size_t);

// conversion
struct core::ConversionOptions { bool apply_lookup = true;
                                 std::optional<std::uint8_t> shade_index{}; };
struct core::PostprocessOptions { bool apply_transparency_fix = true;
                                  bool premultiply_alpha = false;
                                  bool sanitize_matte = false; };
std::expected<core::RgbaImage, core::Error>
core::palette_to_rgba(const core::TileView&, core::PaletteView,
                      core::ConversionOptions = {});
void core::postprocess_rgba(core::RgbaImage&, core::PostprocessOptions = {});

// encoding
std::expected<core::EncodedImage, core::Error>
core::encode_image(core::RgbaImageView, core::ImageFormat,
                   core::EncoderOptions = {});

// adapters
std::expected<std::vector<std::byte>, core::Error>
adapters::read_binary_file(const std::filesystem::path&);
std::expected<void, core::Error>
adapters::write_file(const std::filesystem::path&,
                     std::span<const std::byte> data);
```

## CLI Workflow

1. Read ART and palette bytes using `adapters::read_binary_file`.
2. Load data with `core::load_art` and `core::load_palette`; derive
   `core::PaletteView`.
3. For each tile index:
   - Fetch the tile via `core::get_tile`.
   - Convert with `core::palette_to_rgba` and apply
     `core::postprocess_rgba`.
   - Encode with `core::encode_image` and persist using
     `adapters::write_file`.
4. Surface errors via `core::Error` messages.

## Testing Priorities

- **Art parsing:** synthetic ART blobs exercising header validation, pixel layout
  boundaries, and lookup offsets.
- **Palette loading:** fixtures covering shade tables, translucent data, and
  malformed inputs.
- **Conversion + encoding:** round-trip tests ensuring lookup and shade tables
  change output as expected.
- **Adapters:** maintain GRP parsing coverage and add IO failure scenarios.
- **Batch helper:** verify multi-tile conversion using in-memory assets.

## Documentation & CLI Tasks

- Refresh README/USAGE to document the simplified CLI options.
- Provide code snippets demonstrating direct use of the new modules.
- Note follow-up work for concurrency and additional adapters.

## Follow-ups

- Reintroduce concurrency in the CLI once coverage is in place.
- Add helper utilities to read ART + palette pairs from disk in a single call.
- Provide migration notes for downstream projects that previously included the
  legacy headers.
