# Review Notes

## Repository Survey
- **Languages:** C++20 for core/CLI, CMake for build orchestration, Bash/PowerShell helper scripts, GitHub Actions YAML for CI/CD.
- **Toolchains & Commands:** Primary workflow via `make` targets delegating to `scripts/build/...` wrappers; tests run with `ctest`. Version retrieved through `cmake -P cmake/print_version.cmake`.
- **CI Workflows:** `ci.yml` runs Linux, Windows, macOS builds with duplicated cache/setup steps; `release.yml` mirrors packaging across platforms and generates release notes advertising macOS binaries.
- **Dependencies:** Managed through CPM (cache stored under user temp dirs). Actions install toolchains each run; no compiler cache (`ccache`) or GitHub Actions matrix reuse.
- **Quality Signals:** CI builds all platforms but lacks lint/format stages; release notes mention macOS although delivery is being dropped per request. Build scripts rely on Ninja.
- **Hotspots/Risks:** Redundant workflow steps increase maintenance overhead; caches cover whole build trees risking stale artifacts; release job depends on macOS build; Makefile lacks phony declarations for test-intg variants.

## Immediate Opportunities
- Collapse repeated workflow logic via matrices/composite actions; tighten cache paths to avoid copying build trees.
- Align release messaging with actual platform support (Linux & Windows) and ensure release job tolerates missing macOS artifacts.
- Consider adding compiler caching or targeted dependency install scripts for faster runs.
