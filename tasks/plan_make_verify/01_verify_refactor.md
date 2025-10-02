# 01 Makefile Verify Refactor

## Intent
Simplify and harden the `make verify` workflow by splitting platform checks into reusable targets and improving diagnostics.

## Scope
- `Makefile` verify target and any helper targets/macros it depends on.

Out-of-scope: Modifying build rules for binaries themselves or altering CI workflow logic.

## Acceptance Criteria
- [x] `make verify` delegates to clearly named subtargets (e.g., `verify-linux`, `verify-windows`).
- [x] Linux verification still runs unconditionally; Windows verification runs only when toolchain and binaries are present.
- [x] Command output remains human-friendly with explicit success/failure messages per binary.
- [x] Refactor introduces no redundant rebuilds and keeps compatibility with existing CI pipeline.

## Plan
- Inspect the existing recipe and list repeated checks.
- Extract Linux verification commands into a dedicated phony target.
- Extract Windows verification commands into a guarded phony target, still respecting toolchain detection.
- Update the umbrella `verify` target to call the subtargets and maintain previous behaviour.
- Re-run `make verify` to confirm results.

## Tests
- `make verify`

## Risks & Rollback
- **Risk**: Misordered prerequisites could skip necessary builds; mitigate by keeping `linux` dependency on top-level `verify`.
- **Rollback**: Revert Makefile to prior version if verification fails.

## Evidence
- `make verify`

## Status
Owner: Codex • State: Done • Updated: 2025-10-02
