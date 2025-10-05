# Review Notes

## Repository Survey
- **Languages:** C++20 for core and CLI, CMake for build scripts, doctest for C++ tests.
- **Toolchain:** Build via `make all`, tests with `make test`; CMake integrates CLI11, doctest, fmt, stb.
- **CI Signals:** GitHub Actions workflows cover build/test and release; recent upstream adds lint targets.
- **Hotspots:** `cli/processor.cpp` handles most workflow complexity (parallel exports, progress, error reporting).
- **Risks:** Parallel export uses a thread pool; ensure thread count flags stay consistent across CLI parsing and processing. Merge conflicts likely in CLI files when adding new options.

## Low-Risk Wins
- Keep CLI option-to-processing translation centralized (`make_processing_options`).
- Maintain deterministic directory traversal by sorting collected ART files.
