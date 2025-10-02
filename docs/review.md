# Repository Review — 2025-10-02

## Repo Map
- **Languages**: C++17 (core extractor/library), Bash (CI + local test harness), Make (build orchestration), Dockerfile (container build).
- **Primary binaries**: `src/art2img.cpp` → `bin/art2img`; `src/diagnostic.cpp` → `bin/art2img_diagnostic`.
- **Library surface**: `include/extractor_api.hpp` exposes an embeddable API around `ArtExtractor`/`Palette`.
- **Build/test commands**: `make` / `make linux` (local), `make windows` (MinGW cross), `make build-all`, `make verify`, `make test`, `./tests/test_ci.sh`, `docker build -t art2img .`, `./build.sh` (all-in-one, Linux-first, optional Windows).
- **Tooling**: GNU Make, GCC/MinGW (`-std=c++17 -O2 -pthread`), vendored CLI11 parsing, BS_thread_pool for concurrency, stb_image_write for PNG/TGA output; tests use shell scripts plus fixture ART assets under `tests/assets`.

## Dependency Health
- **CLI11 2.5.0** (BSD-3-Clause, 2023-08) — latest public release is 2.4.2 (2023-02); 2.5.0 is a dev snapshot without tags. Consider pinning to an official tag to avoid drift.
- **stb_image_write v1.16** (Public Domain/MIT, 2021-09): upstream bugfix v1.17 (2023-04) adds CVE-2023-4808 guardrails; worth upgrading.
- **BS_thread_pool (header)** (MIT, 2020-05 last tagged 3.3.0): vendored copy lacks version marker; upstream has fixes for thread affinity & exception safety in 2022—verify whether we need them or pin to tag.
- **System deps**: `g++`, `x86_64-w64-mingw32-g++` for Windows builds, `file`, `nm`, `ldd` used in CI. No package manager lockfiles; vendored headers avoid runtime downloads.

## Quality Signals
- **Build**: `make linux` succeeds locally; `make windows` relies on MinGW availability.
- **Tests**: `make test` (20s, heavy asset extraction + PNG memory regression) and `tests/test_ci.sh` (shell-based smoke + CLI regression). No unit tests or coverage metrics. No sanitizers/valgrind runs.
- **Formatting/Lint**: No clang-format/clang-tidy integration; inconsistent include ordering (some headers rely on transitive includes).
- **CI**: `.github/workflows/build.yml` matrix (linux/windows cross). Current workflow fails at verification due to incorrect diagnostic binary name (`art_diagnostic` vs `art2img_diagnostic`). No caching; repeated full builds.

## Hotspots & Observations
- **`src/extractor.cpp`**: heavy coupling (filesystem cleanup, threading, PNG/TGA dispatch) in single method; deletes output dir recursively before extraction—risky if pointed at shared path.
- **`src/palette.cpp`**: massive static palette tables; `load_from_file` throws on open failure despite boolean return, making fallback logic in CLI brittle.
- **`src/png_writer.cpp`**: STL + stb interplay; lambda in `write_png_to_memory` overwrites buffer instead of appending (data loss bug under multi-chunk writes).
- **`tests/test_functionality.sh`**: long-running integration script, duplicates logic with CI script; output directories wiped each run.
- **Header hygiene**: `include/extractor.hpp` uses `std::thread` without including `<thread>`; relies on indirect includes.

## Risks & Unknowns
- **Pipeline breakage**: Release matrix exits with status 1 because verification step references `bin/art_diagnostic*`; prevents releases.
- **In-memory PNG extraction**: Current implementation loses data for images > single chunk (stb writes 128k blocks). Library API consumers likely get truncated images.
- **Palette loading**: CLI fallback assumes lowercase `palette.dat`; fails silently on case-sensitive filesystems, then `load_from_file` throws if a user-specified invalid path survives `resolve_palette_path` heuristics.
- **Directory cleanup**: Blind `std::filesystem::remove_all(output_dir)` can erase pre-existing assets if misconfigured; no dry-run or confirmation.
- **Vendored deps**: No documented update cadence; lack of LICENSE file in release packages flagged in workflow (`cp LICENSE` best-effort echo).

## Immediate Low-Risk Wins
- Fix CI verification script to reference `art2img_diagnostic` binaries.
- Patch `PngWriter::write_png_to_memory` to append chunks and preserve buffer contents.
- Add explicit `<thread>` include (and other required headers) in public headers; run clang-tidy/include-what-you-use to avoid accidental build breaks.
- Adjust `Palette::load_from_file` to return `false` on missing file instead of throwing, aligning with CLI fallback strategy; surface clear CLI error when all palette lookups fail.
- Harden `Makefile`/`verify` target to skip Windows rebuild when cross toolchain absent (so local `make verify` never fails spuriously).
- Document canonical build/test commands plus safety notes in `docs/plan.md` (currently misses agent contract expectations).
