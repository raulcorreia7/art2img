# 01 PNG Memory Buffer Integrity

## Intent
Ensure `PngWriter::write_png_to_memory` preserves complete PNG output even when stb emits multiple chunks, protecting API consumers from truncated buffers.

## Scope
- `src/png_writer.cpp`
- `include/png_writer.hpp` (if signature or docs need clarification)
- `tests/test_library_api.cpp` or new regression harness

Out-of-scope: altering on-disk PNG format or CLI pathways.

## Acceptance Criteria
- [x] In-memory writer accumulates all callback chunks without data loss.
- [x] Regression test fails against current main branch but passes after fix.
- [x] CLI output for representative tiles remains byte-identical to pre-fix binaries.
- [x] No new dependencies introduced.

## Plan
- Reproduce truncation via synthetic large tile (trigger multiple stb callbacks).
- Patch write callback to append rather than overwrite, optionally record chunk count for debugging.
- Extend library API test (or add new one) to validate PNG buffer size > 0 and matches file-based output.
- Run `make test` and targeted regression command; capture outputs.

## Tests
- `make test`
- Dedicated regression binary/test (to be documented in Evidence section)

## Risks & Rollback
- **Risk**: stb callback semantics may differ across platforms — mitigate by keeping append logic minimal and relying on unit test.
- **Rollback**: Revert PNG writer changes and disable new regression test.

## Evidence
- `make test` (adds PNG memory regression coverage)
- `./tests/test_ci.sh`

## Status
Owner: Codex • State: Done • Updated: 2025-10-02
