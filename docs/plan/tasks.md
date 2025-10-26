# art2img — Implementation Status (Memory-First API)

All milestones for the promoted `art2img::core` API are complete. The table
below documents the state of the simplified codebase after removing the legacy
headers and sources.

## Milestone Summary

| Milestone | Status | Notes |
|-----------|--------|-------|
| Project scaffold | ✅ Complete | CMake project with `include/`, `src/`, `tests/`, and CLI target. |
| Error handling | ✅ Complete | `core::Error`, `errc`, helper factories, doctest coverage. |
| Palette loader | ✅ Complete | `core::load_palette`, `Palette`, `PaletteView`, shade/translucent support. |
| ART archive | ✅ Complete | `core::load_art`, `ArtArchive`, `TileView`, lookup handling. |
| Conversion & post-processing | ✅ Complete | `core::palette_to_rgba`, `postprocess_rgba`, `ConversionOptions`, `PostprocessOptions`. |
| Encoding | ✅ Complete | `core::encode_image`, PNG/TGA/BMP support via stb headers. |
| IO & adapters | ✅ Complete | `adapters::read_binary_file`, `write_file`, `load_grp`, manifest formatters. |
| Extras | ✅ Complete | `extras::convert_tiles` orchestrating archive/palette/encode flows. |
| CLI | ✅ Complete | Memory-first pipeline with simplified flags and explicit palette input. |
| Documentation | ✅ Complete | README/USAGE/architecture/iteration notes updated to new API. |
| Legacy cleanup | ✅ Complete | Removed `include/art2img/*.hpp` legacy headers, old `src/*.cpp`, outdated tests. |

## Outstanding Follow-Ups

- Reintroduce focused doctest suites for `core::load_art`, `core::load_palette`,
  conversion/encoding edge cases, and batch helpers.
- Add regression coverage for CLI argument parsing and error handling.
- Explore concurrency reintroduction once coverage is restored.
