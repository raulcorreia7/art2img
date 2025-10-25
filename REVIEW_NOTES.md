# Review Notes

## Repository snapshot
- **Language/toolchain**: Modern C++23 library built with CMake; depends on stb (vendored through CPM) and doctest for tests.
- **Primary targets**: `art2img_core` static/shared library plus optional CLI/tests via CMake presets (`make build`, `ctest`).
- **Design intent**: Memory-first API promoted under `art2img::core`, with optional adapters/extras on top.

## Key findings
1. **"New" memory-first API is still a thin wrapper over the legacy namespace**  
   Headers in `include/art2img/core/` include the legacy public headers and store back-pointers to `::art2img` types (e.g. `TileView::legacy_`, `Palette::backing_`), while implementations call the old entry points (`::art2img::load_art_bundle`, `::art2img::to_rgba`, `::art2img::encode_image`). This keeps all legacy structures alive and duplicates the public surface without moving behaviour into the new modules.
2. **Coupling forces duplicate allocations and muddles ownership**  
   `core::load_art` copies the entire legacy `ArtData` into a shared pointer so that tiles can keep referring to the original views, effectively storing the ART payload twice (raw span + legacy buffers). Palette loading performs a similar copy, so memory-first callers still pay for the legacy owning containers even when they only need spans.
3. **New namespaces remain untested**  
   The doctest suites continue to exercise the legacy `art2img` namespace; nothing in `tests/` covers the promoted `art2img::core/adapters/extras` contracts, leaving the new API without direct regression coverage.
4. **Directory layout keeps both legacy and promoted modules side-by-side**  
   The `include/art2img/` tree still exposes the previous modular folders (`convert/`, `palette/`, etc.) alongside the new `core/`, so consumers see two competing hierarchies. This undermines the effort to provide a single intuitive surface.
5. **Extras/adapters inherit legacy complexity**  
   Batch helpers and I/O adapters simply forward into the legacy namespace, adding more layers without simplifying options or error handling. The manifest formatter duplicates logic instead of sharing a composable serialization utility.

## Cleanup & refactor proposal
- **Collapse behaviour into the new modules**: Move ART parsing, palette loading, and encoding logic into the `art2img::core` implementations directly and reduce legacy headers to thin forwarding stubs (or deprecate them). This eliminates `detail_access` backdoors and shared_ptr bridges.
- **Simplify data ownership**: Represent archives and palettes purely in terms of spans/containers owned by the new structs; keep optional shared ownership only when streaming from legacy adapters. Avoid duplicating buffers where the caller already supplies the data.
- **Retire redundant public folders**: Once behaviour lives under `core/adapters/extras`, demote the legacy headers to compatibility wrappers in a `legacy/` subdirectory (or remove them in the breaking release) so the include tree matches the published design.
- **Add focused tests for the promoted API**: Mirror the legacy doctests with new suites that exercise `core::load_art`, `core::palette_to_rgba`, `adapters::load_grp`, and `extras::convert_tiles` using in-memory fixtures. This ensures the refactored code is covered before removing the fallback layers.
- **Re-evaluate helpers after the port**: With the core rewritten, prune or simplify extras/adapters that no longer add value (e.g. expose a lightweight manifest builder instead of formatting in place).

