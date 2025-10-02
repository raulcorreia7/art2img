# 01 Thread Defaults & Palette Diagnostics

## Intent
Prevent zero-thread extraction failures and provide clearer palette resolution feedback without altering CLI behaviour.

## Scope
- `include/extractor.hpp`
- `src/art2img.cpp`
- `src/extractor.cpp` (if validation helpers required)

Out-of-scope: redesigning extraction scheduling or palette heuristics.

## Acceptance Criteria
- [x] Default thread count clamps to ≥1 even when `std::thread::hardware_concurrency()` reports zero.
- [x] Headers include required standard library dependencies explicitly.
- [x] CLI emits actionable message when palette resolution exhausts all fallbacks.
- [x] All existing tests continue to pass without changes to CLI interface.

## Plan
- Add `<thread>` include and helper to sanitize hardware concurrency (both in CLI and library options).
- Update CLI palette handling to log the final resolved path or explicit failure before defaulting.
- Exercise `make test` to verify CLI still works; optionally add focused smoke invocation.

## Tests
- `./bin/art2img --help` (sanity)
- `make test`

## Risks & Rollback
- **Risk**: Additional logging may disturb tests expecting exact output — mitigate by limiting messaging to verbose/quiet-aware paths.
- **Rollback**: Revert changes to extractor options and CLI logging.

## Evidence
- `make test`
- `./tests/test_ci.sh`

## Status
Owner: Codex • State: Done • Updated: 2025-10-02
