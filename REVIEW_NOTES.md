# Review Notes

## Repository snapshot
- **Language/toolchain**: Modern C++23 library built with CMake; stb headers are
  fetched via CPM, doctest is used for unit tests.
- **Primary targets**: `art2img_core` library, lightweight CLI (`cli/main.cpp`),
  and doctest-based suites under `tests/unit`.
- **Design intent**: Memory-first `art2img::core` namespace drives ART parsing,
  palette handling, conversion, and encoding. `adapters` provide file/GRP IO and
  manifest helpers; `extras` expose batch utilities.

## Current state
1. **Legacy surface removed**  
   The previous `include/art2img/*.hpp` hierarchy and the associated source
   files were deleted. Only the promoted `core`, `adapters`, and `extras`
   modules remain, so consumers always interact with the memory-first API.
2. **Error handling unified**  
   `core::Error` now owns the error category, helper factories, and formatting
   utilities. All modules use `core::errc` directly and no longer depend on the
   legacy namespace for diagnostics.
3. **CLI aligned with core modules**  
   The CLI loads resources through `adapters::read_binary_file`, performs
   conversion via `core::palette_to_rgba`, applies post-processing in place, and
   encodes images through `core::encode_image`. Animation export and directory
   recursion were removed to keep the workflow focused and predictable.
4. **Documentation refreshed**  
   `USAGE.md` now documents the simplified CLI and shows how to wire the new
   modules together from C++.
5. **Tests trimmed, coverage follow-up required**  
   Legacy doctest suites that targeted the removed headers were dropped. Only
   the error-formatting checks and the GRP adapter test remain; new coverage for
   `core::load_art`, palette conversion, and batch helpers should be added in a
   follow-up.

## Follow-up recommendations
- Rebuild unit coverage around the new `core` contracts, starting with
  `load_art`, `load_palette`, `palette_to_rgba`, and `encode_image` edge cases.
- Extend the CLI with optional concurrency once the simplified pipeline is
  stable and covered by tests.
- Consider providing small adapter helpers for common workflows (e.g. load ART
  + palette from disk) so CLI and applications share a single code path.
