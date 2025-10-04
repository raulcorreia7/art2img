# AGENTS.md — Autonomous Development Contract

Authoritative, minimal contract for autonomous work. Optimize for simple, clean, modular, maintainable code. Use proven patterns; avoid over-engineering. Modes: Restate → Review → Plan → Build → Test → Verify → Self-Reflect.

---

## Quick Start
```bash
make setup
make all
````

---

## Agent Modes

### REVIEW MODE

Capture a concise review process:

* Languages, toolchains, entry points, build/test/release commands.
* Dependency health: versions, cadence, licenses, advisories, upgrade candidates.
* Quality signals: lint/format status, test count/coverage (if available), CI duration/caches, flaky/slow tests.
* Hotspots: large/complex modules, duplication clusters, dependency cycles, perf bottlenecks, portability issues.
* Risks/unknowns and immediate low-risk wins.
* Code: Code that is complex, coupled, duplicated work, check composability, modularity, simplicity. Promote ease-of-use, simplicity, organic.

Output location: after the review, output to the console and write to review.md

### PLAN MODE

Produce a concrete plan and a decomposed set of atomic tasks:

* Prefer **agentic tools** for repository reconnaissance (symbol graphs, usages, cross-repo search). Fall back to **system tools** when faster for code search or structure inventory (see “Tooling Policy”). ([ctags.io][1])
* Minimize questions; make labeled assumptions with explicit risks, unless critical findings are found.
* Define acceptance criteria, test strategy, and rollback.
* Ask if breaking changes or backwards compability is requested.
* Draw just enough diagrams to augument the plan mode, choose the output style that fits the agentic tool.

Represent the plan directly in the PR description (checklist of tasks and acceptance criteria).

### BUILD MODE

Deterministic, minimal diffs:

* Execute implementation steps in the order specified by the plan.
* Keep changes small and reversible; update tests alongside code.
* Prefer stable, community-reviewed libraries; follow upstream install docs; integrate latest stable, then lock with the ecosystem’s lockfile (e.g., `uv` for Python). ([Docker Documentation][2])
* For multi-file edits: use agentic batch edits when available; otherwise use non-interactive system tools (grep/rg + sed/awk + ctags) to refactor safely at scale. ([GitHub][3])

### TEST MODE

Ensure all implemented changes are covered:

* Unit tests for utilities; integration tests for workflows;
* Prefer real fixtures/assets over mocks; generate realistic fixtures when real data is unavailable.
* Treat performance/memory budgets as tests where feasible.
* Ensure features are covered with happy paths, and edge cases.

### VERIFY MODE

Post-change checks before merge:

* Build, linting, formatting tools pass.
* Smoke tests pass; CI green on supported OS/toolchains; artifacts produced for review.
* Container images (if any) built via multi-stage Dockerfiles; final images minimal, non-root where possible. ([Docker Documentation][4])

### SELF-REFLECT MODE

Confirm the change satisfies objectives and acceptance criteria, adheres to this contract, and leaves no dead code or stale comments. Note residual risks and follow-ups in the PR.

End of a plan/task do a high level summary what was done with just enough details.

---

## Tooling Policy

**Agentic (primary in Plan/Build)**

* Your code assistant of choice for project-wide refactors and explanations (keep loops short, checkpoint often).

**System (adjacent/fallback)**

* ripgrep (`rg`) for fast, gitignore-aware search. ([GitHub][3])
* fzf for interactive filtering/selection. ([GitHub][5])
* grep for ripgrep fallback, to find references.
* gnu find, to find files.
* ls to list directories of multiple files.

Useful commands:

```bash
rg -n --no-messages 'TODO|FIXME|HACK'
rg -n --stats -S 'class |struct |interface |def ' -g '!**/vendor/**'
ctags -R -f .tags .
```

---

## Build & Containers

Expose canonical dev/build/test commands in the root `README`. For containers, use multi-stage Dockerfiles to keep runtime images small and reduce attack surface; copy only required artifacts into the final image. ([Docker Documentation][2])

---

## Coding Standards

**Primary language**

* Enforce the ecosystem’s standard style and formatter. Keep linters fast. Public APIs are typed or interface-annotated.
* Promote for best practices.
* Tests: unit for utilities; integration for workflows;

**Code Guidelines**

* Keep changes clean, modular, avoid complexity, organic, easy to read/understand/use.
* Promote patterns, but don't over do it, use a goldielocks approach, when it best suits, else promote simplicity and avoid complexity.

**Shell**

* `set -euo pipefail`; trap errors; keep shell thin; prefer the primary language for logic.
* Promote the usage of functions to perform logic

**Make**

* Idempotent targets; explicit inputs/outputs; minimal `.PHONY`.
* Try to have best makefile practices, all/build/test/clean

**Pipelines**

* Idempotent steps, pre/post validation, deterministic ordering, cross-platform behavior, resumable design (state files as needed).

**Prose**

* Professional, concise. No emojis.

---

## Dependencies and Locking

Adopt → lock → verify.

* Use the platform’s lockfile and reproducible install. Example for Python via `uv`:

```bash
uv pip compile pyproject.toml -o requirements.lock
uv pip sync requirements.lock
```

([Docker Documentation][2])

---

## Testing & CI

* Run unit and integration suites on each feature implementation; add property-based tests where useful; enforce performance/memory budgets when stability matters.
* CI: matrix across relevant OS/toolchains; cache dependency/compile artifacts (e.g., ccache/sccache); publish build artifacts for review. ([Docker Documentation][4])

---

## Change Management

* Small, single-concern commits and PRs.
* Conventional Commits for messages; link tasks in the PR checklist; include repro steps, risks, and evidence. ([conventionalcommits.org][7])

---

## Security

* No network writes without explicit approval.
* Validate all external inputs; sanitize paths; avoid unsafe deserialization.
* Secret scanning in CI; minimal runtime images; prefer non-root execution. ([Docker Documentation][4])
* Validate/Review dependencies are latest stable when applicable.

## Documentation
* Keep documentation up to date, clean, consise, professional, and LLM friendly.
* Ensure documentation is always up-to-date with all changes implementation in the application.
* Review if the documentation is up to date/relevant.

```
::contentReference[oaicite:13]{index=13}
```

[1]: https://ctags.io/?utm_source=chatgpt.com "Home · Universal Ctags"
[2]: https://docs.docker.com/build/building/multi-stage/?utm_source=chatgpt.com "Multi-stage builds"
[3]: https://github.com/BurntSushi/ripgrep?utm_source=chatgpt.com "ripgrep recursively searches directories for a regex pattern ..."
[4]: https://docs.docker.com/build/building/best-practices/?utm_source=chatgpt.com "Building best practices"
[5]: https://github.com/junegunn/fzf?utm_source=chatgpt.com "junegunn/fzf: :cherry_blossom: A command-line fuzzy finder"
[6]: https://github.com/universal-ctags/ctags?utm_source=chatgpt.com "universal-ctags/ctags: A maintained ctags implementation"
[7]: https://www.conventionalcommits.org/en/v1.0.0/?utm_source=chatgpt.com "Conventional Commits"
