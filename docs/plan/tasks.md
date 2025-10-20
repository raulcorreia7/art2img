# art2img vNext — Implementation Roadmap (LLM Friendly)

Every task below is self-contained and written so an autonomous code agent can execute it end-to-end. Tasks are grouped into milestones that must land in order. Each task includes inputs, actions, outputs, acceptance criteria, and any diagrams or flow hints needed. All paths are relative to repository root.

---

## Legend

- **Inputs** – Files/resources the agent must inspect or move.
- **Actions** – Ordered list of subtasks. Follow sequentially unless otherwise stated.
- **Outputs** – Files/artifacts that must exist or be updated when the task completes.
- **Acceptance** – Validation steps (tests, lints, checks) to run.
- **Notes** – Warnings, heuristics, or diagrams.

---

## Milestone 1 — Project Setup

### T1.0 – Scaffold project tree
- **Inputs:** `docs/plan/architecture.md`, `docs/plan/tasks.md`.
- **Actions:**
  1. Create directories: `include/art2img`, `src`, `tests`, `cmake` scaffolding for new core.
  2. Prepare minimal `CMakeLists.txt` referencing upcoming modules, set `CMAKE_CXX_STANDARD 23`.
  3. Add placeholder README note referencing ongoing implementation.
- **Outputs:** Skeleton tree ready for new modules (`include/art2img/*.hpp`, `src/*.cpp`, `tests/*`).
- **Acceptance:** Root `CMakeLists.txt` configures without errors (even if no targets yet).


---

## Milestone 2 — Core Infrastructure

### T2.1 – Implement `types.hpp`
- **Inputs:** Architecture section 4, constants inventory.
- **Actions:**
  1. Define palette/image constants in `include/art2img/types.hpp`.
  2. Add unit test verifying constants when necessary.
- **Outputs:** `include/art2img/types.hpp`, optional `tests/core/types/test_constants.cpp`.
- **Acceptance:** Test compile (no runtime tests needed yet).

### T2.2 – Implement `error.hpp`
- **Inputs:** Architecture section 4.1.
- **Actions:**
  1. Define `errc` enum, custom error category, `Error` struct, helper functions returning `std::expected<std::monostate, Error>` failure objects.
  2. Add unit tests for error code conversion, message formatting.
- **Outputs:** `include/art2img/error.hpp`, `src/error.cpp`, tests under `tests/core/error`.
- **Acceptance:** Unit tests pass (`ctest` subset).

### T2.3 – Hook core target in CMake
- **Inputs:** New headers (`types.hpp`, `error.hpp`).
- **Actions:**
  1. Create `art2img_core` library target exporting headers.
  2. Add install/export rules and interface compile definitions.
- **Outputs:** Updated `CMakeLists.txt` with `art2img_core` target.
- **Acceptance:** `cmake --build` (configured) succeeds.

---

## Milestone 3 — Palette Module

### T3.1 – Implement palette loader
- **Inputs:** Architecture sections 4.2, 9 (palette notes), sample assets (e.g., `tests/assets/PALETTE.DAT`).
- **Actions:**
  1. Write `include/art2img/palette.hpp` API definitions.
  2. Implement `src/palette.cpp` loading from path/span, parsing raw RGB + shade tables + translucent map.
  3. Provide utility functions for palette entry conversion.
 4. Enforce validation rules from Architecture §14 (reject truncated payloads, clamp shade indices).
 5. Write unit tests with real + corrupted PALETTE files.
- **Outputs:** Header, source, tests (`tests/palette/test_palette.cpp`), fixture referencing test asset path.
- **Acceptance:** `cmake --build --target tests` + `ctest -R palette` pass.
- **Notes:** Use `std::filesystem`, `std::span`, `std::expected`. Validate size before reading; map errors to `errc::invalid_palette`.

### T3.2 – Document palette module usage
- **Actions:** Update `docs/plan/architecture.md` (if needed) and add inline comments summarizing palette behaviour.
- **Outputs:** Possibly new doc snippet or inline doc comments.
- **Acceptance:** Documentation reflects final API.

---

## Milestone 4 — ART Module

### T4.1 – Implement `art.hpp` + loader
- **Inputs:** Architecture sections 4.3, 9 (art notes), assets `TILES*.ART`, `LOOKUP.DAT`.
- **Actions:**
  1. Define `TileAnimation`, `TileView`, `ArtData`, `PaletteHint` in header.
  2. Implement loader in `src/art.cpp` for path/span: parse header, metadata, allocate contiguous buffers, build `TileView`s referencing `pixels`/`remaps`.
 3. Implement optional palette hint logic (auto-discover sidecar palette/lookup path resolution separated into helper).
 4. Enforce safety checks from Architecture §14 (offset bounds, width*height consistency, lookup sizes).
 5. Add unit tests for valid/corrupt ART files, verifying metadata, view spans, animation bits.
- **Outputs:** `include/art2img/art.hpp`, `src/art.cpp`, tests `tests/art/test_art_loader.cpp`.
- **Acceptance:** `ctest -R art` passes; memory spans validated (no out-of-range).
- **Notes:** Compare tile ranges to architecture doc; error -> `errc::invalid_art` or `errc::io_failure`.

### T4.2 – Helpers for tile iteration
- **Actions:** Provide helper functions if needed (within `art.cpp`) for `make_tile_view`, tile lookup by Build ID.
- **Outputs:** Additional functions, doc updates.
- **Acceptance:** Tests covering helper behaviour.

---

## Milestone 5 — Conversion Module

### T5.1 – Implement `convert.hpp/.cpp`
- **Inputs:** Architecture section 4.4 & 9.
- **Actions:**
  1. Define `ConversionOptions`, `Image`, `ImageView`, `ColumnMajorRowRange` in header.
 2. Implement `to_rgba`, row-major copy, sampling, row iterator. Handle palette remap, shade, transparency, optional premultiplication while obeying bounds checks from Architecture §14.
 3. Add unit tests verifying pixel parity with known tiles (use fixtures) and edge cases (empty tile, remap errors, out-of-bounds sampling).
- **Outputs:** Header, source, tests `tests/convert/test_convert.cpp`.
- **Acceptance:** `ctest -R convert` passes; memory sanitizers optional.

### T5.2 – Performance sanity check
- **Actions:** Optional benchmarking harness to document conversion performance.
- **Outputs:** Bench report (Markdown or test comment) summarizing findings.
- **Acceptance:** No regressions observed (qualitative).

---

## Milestone 6 — Encoding & IO

### T6.1 – Implement `encode.hpp/.cpp`
- **Inputs:** Architecture section 4.5, 9 (encode notes), stb library (from dependencies).
- **Actions:**
  1. Define format enum + option structs + EncodeOptions variant.
 2. Implement encode functions using stb; validate stride/channel metadata before calling stb and map failures to `errc::encoding_failure`.
 3. Add tests verifying encoded output (size, basic headers) and error paths (invalid stride, unsupported format).
- **Outputs:** Header, source, tests `tests/encode/test_encode.cpp` (use small fixture Image).
- **Acceptance:** `ctest -R encode` passes.

### T6.2 – Implement `io.hpp/.cpp`
- **Inputs:** Architecture section 4.5.
- **Actions:**
  1. Provide file read/write helpers returning `std::expected`.
  2. Tests for success, missing file, permission errors (simulate with temporary dir/umask).
- **Outputs:** Header, source, tests `tests/io/test_io.cpp`.
- **Acceptance:** `ctest -R io` passes.

---

## Milestone 7 — Public API & CLI

### T7.1 – Publish `api.hpp`
- **Actions:** Create header including `types`, `error`, `palette`, `art`, `convert`, `encode`, `io`.
- **Outputs:** `include/art2img/api.hpp`.
- **Acceptance:** Build compiles with single include.

### T7.2 – Rebuild CLI on new pipeline
- **Inputs:** Architecture section 6, CLI design from docs/plan.
- **Actions:**
  1. Create new CLI entry under `cli/` referencing `art2img/api.hpp`.
  2. Implement option parsing (reuse CLI11 dependency), translation into pipeline steps.
  3. Use thread pool (optional) around tile conversion/encoding; ensure sequential fallback works.
  4. Update CLI integration tests to use new binary.
- **Outputs:** `cli/main.cpp`, support files, tests under `tests/cli`.
- **Acceptance:** `ctest -R cli`, manual CLI run on sample ART.

### T7.3 – Update build scripts for new CLI
- **Actions:** Refresh `Makefile`, `scripts` (if needed) to point to new build targets.
- **Outputs:** Updated automation referencing new CLI.
- **Acceptance:** `make all`, `make test` succeed.

---



## Milestone 8 — Cleanup & Documentation

### T8.1 – Cleanup
- **Actions:** Remove redundant code and ensure clean tree structure.
- **Outputs:** Clean tree referencing only new modules.
- **Acceptance:** `git status` shows removal of unused artifacts.

### T8.2 – Documentation pass
- **Actions:** Update README, CLI usage instructions, dependency notes.
- **Outputs:** Revised docs reflecting new API.
- **Acceptance:** Docs render cleanly; cross-links valid.

### T8.3 – Final QA sweep
- **Actions:**
  1. `cmake --build` for release + debug configurations.
   2. Run full test suite: `ctest`, CLI smoke.
  3. Run static analysis (`clang-tidy` profile) and sanitizers (ASAN/UBSAN/LSAN) per Architecture §14.
  4. Package artifacts if part of release flow.
- **Outputs:** QA summary (notes or CI logs) demonstrating green pipeline and clean analysis/sanitizer runs.
- **Acceptance:** All commands succeed; static analysis/sanitizers report zero findings; summary recorded in PR or release notes.

---

## Visual Roadmap

```mermaid
gantt
    dateFormat  YYYY-MM-DD
    title  art2img Implementation Roadmap

    section Project Setup
    T1.0 Project Skeleton :active,  des1, 2024-07-01, 1d

    section Core Infra
    T2.1 Types/Error     :        des2, after des1, 1d
    T2.2 CMake Core      :        des3, after des2, 1d
    T2.3 Core Target     :        des4, after des3, 1d

    section Palette/Art
    T3.1 Palette Loader  :        des5, after des4, 2d
    T3.2 Palette Docs    :        des6, after des5, 1d
    T4.1 Art Loader      :        des7, after des6, 3d
    T4.2 Art Helpers     :        des8, after des7, 1d

    section Convert/Encode/IO
    T5.1 Convert Module  :        des9, after des8, 3d
    T5.2 Convert Perf    :        des10, after des9, 1d
    T6.1 Encode Module   :        des11, after des10, 3d
    T6.2 IO Module       :        des12, after des11, 2d

    section API & CLI
    T7.1 API Barrel      :        des13, after des12, 1d
    T7.2 CLI Rebuild     :        des14, after des13, 3d
    T7.3 Build Scripts   :        des15, after des14, 1d

    section Cleanup
    T8.1 Cleanup         :        des16, after des15, 1d
    T8.2 Documentation   :        des17, after des16, 2d
    T8.3 Final QA        :        des18, after des17, 2d
```

---

## Build/Test Command Cheat Sheet

- Configure: `cmake -S . -B build`
- Build: `cmake --build build`
- Tests: `cd build && ctest --output-on-failure`
- Sanitizers: `cmake -S . -B build-asan -DENABLE_ASAN=ON -DENABLE_LEAK_SANITIZER=ON`
- CLI smoke: `./build/bin/art2img --help`

---

## Dependencies & Assets

- Assets remain under `tests/assets` for reuse in new tests.
- External libs via CPM: CLI11 v2.5.0, doctest 2.4.12, fmt 11.0.2, stb (tagged release).
- Thread pool: evaluate reuse of `BS_thread_pool` or switch to standard `<execution>` once stable (optional).

---

## Acceptance Gate (Final)

To mark the implementation complete:
1. New pipeline (`api.hpp` and modules) is the default include path.
2. CLI uses new pipeline exclusively.
3. Documentation, build scripts, examples updated.
4. Full QA (build + test + sanitizers) passes.
