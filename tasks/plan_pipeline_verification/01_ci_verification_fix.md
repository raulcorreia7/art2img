# 01 CI Verification Fix

## Intent
Repair the GitHub Actions verification stage so release builds no longer fail when verifying diagnostic binaries.

## Scope
- `.github/workflows/build.yml`
- `Makefile` verify target (guardrails only)

Out-of-scope: changing CI matrix composition or build artefact packaging.

## Acceptance Criteria
- [x] Workflow references correct binary paths for both Linux and Windows jobs.
- [x] `make verify` succeeds locally even when MinGW toolchain is absent.
- [x] CI step aborts with clear message if Windows binaries are missing after build stage.
- [x] Documentation in `docs/plan.md` remains aligned with new verification flow.

## Plan
- Inspect existing verify step to map current binary names and failure conditions.
- Rename workflow checks to use `art2img_diagnostic` naming (incl. Windows suffix handling).
- Add guard to `make verify` so `_windows` rebuild runs only when binaries are missing and toolchain exists.
- Smoke-run `make verify` locally; capture expected output for plan/test evidence.

## Tests
- `make verify`

## Risks & Rollback
- **Risk**: Workflow edits may break archive globbing — mitigate by re-running `make verify` locally and inspecting `bin/` layout.
- **Rollback**: Revert workflow + Makefile changes and re-run previous pipeline.

## Evidence
- `make verify`

## Status
Owner: Codex • State: Done • Updated: 2025-10-02
