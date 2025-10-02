````md
# AGENTS.md — Autonomous Development Contract

Authoritative, minimal contract for autonomous work in this repository. Keep code simple, clean, modular, and maintainable. Use proven patterns. Modes: Review → Plan → Build → Verify.

---

## Quick Start
```bash
make setup
make all
````

---

## Agent Modes

### REVIEW MODE

Create `docs/review.md` when code exists. Capture:

* Repo map: languages, toolchains, entry points, build/test/release commands.
* Dependency health: versions, cadence, licenses, advisories, upgrade candidates.
* Quality signals: lint/format status, test count/coverage (if available), CI duration/caches, flaky/slow tests.
* Hotspots: large/complex modules, duplication clusters, cycles, perf bottlenecks, portability issues.
* Risks/unknowns; immediate low-risk wins.

### PLAN MODE

Create `docs/plan.md`, `tasks/status.md`, and atomic tasks under `tasks/plan_<slug>/`.

* Tooling rule: prefer **agentic tools** for repository reconnaissance (symbol graph, usages, cross-repo search). Fall back to **system tools** when they’re faster for code search or structural inventory. Agentic examples: Sourcegraph Cody; Claude Code. System examples: ripgrep, fzf, universal-ctags. ([Sourcegraph Docs][1])
* Minimize questions; make labeled assumptions. Record risks, external dependencies, rollback paths.

### BUILD MODE

Deterministic changesets. Execute `DIRECTIVE` blocks in order. Small diffs. Update docs/tests in the same change. Append `.agents/action_log.md`.

* Prefer stable, community-reviewed libraries; verify install from upstream README. Integrate latest stable, then lock with the ecosystem’s lockfile (e.g., `uv` for Python projects). ([Sourcegraph][2])

### VERIFY MODE

Post-change smoke and performance checks. CI green; artifacts published. Update ADRs if architecture changed (use MADR template or equivalent). ([GitHub][3])

---

## Tooling Policy

**Agentic tools (primary in Plan/Build)**

* Sourcegraph Cody for code search across repos, symbol-aware context, large-scale edits. ([Sourcegraph Docs][1])
* Claude Code for project-wide assistance and autonomous edit workflows. ([Business Insider][4])

**System tools (adjacent/fallback)**

* ripgrep (`rg`) for fast, gitignore-aware search. ([GitHub][5])
* fzf for interactive filtering/selection. ([GitHub][6])
* universal-ctags for tags/symbol index. ([GitHub][7])

**Useful commands**

```bash
rg -n --no-messages 'TODO|FIXME|HACK'
rg -n --stats -S 'class |struct |interface |def ' -g '!**/vendor/**'
ctags -R -f .tags .
```

---

## Build & Containers

Expose canonical dev/build/test commands in `docs/plan.md`. For containerized builds, prefer multi-stage Dockerfiles to produce minimal runtime images; copy only the artifacts you need. ([Docker Documentation][8])

---

## Repository Layout

```
docs/
  review.md
  plan.md
  decisions/               # ADRs 0001-....md
tasks/
  status.md
  plan_<slug>/
    01_task-name.md
    02_task-name.md
.agents/
  memory_bank.md           # durable conventions/commands
  action_log.md            # append-only timestamps + metrics
  directives/              # reusable DIRECTIVE snippets
inputs/                    # read-only (gitignored)
outputs/                   # write-only (gitignored)
.cache/                    # tool caches (gitignored)
.state/                    # state.json, resumable markers (gitignored)
```

---

## Coding Standards

**Primary language**

* Enforce the ecosystem’s standard style and formatter. Keep linters fast. Type hints or interfaces on public APIs. Tests: unit for utilities; integration for workflows; snapshot/golden for binaries with tolerance windows.

**Shell**

* `set -euo pipefail`; trap errors; keep shell thin; prefer main language for logic.

**Make**

* Idempotent targets; explicit inputs/outputs; minimal `.PHONY`.

**Pipelines**

* Idempotent steps, pre/post validation, deterministic ordering, cross-platform behavior, resumable via `.state/state.json`.

**Docs**

* Machine-consumable first: commands, paths, contracts in `docs/plan.md` and `.agents/memory_bank.md`.

---

## Dependencies and Locking

Adopt → lock. Use the platform’s lockfile and reproducible install. Example for Python projects using `uv`:

```bash
uv pip compile pyproject.toml -o requirements.lock
uv pip sync requirements.lock
```

([Sourcegraph][2])

---

## Testing & CI

* Unit + integration on each change; property-based where useful; perf/memory budgets as tests where feasible.
* CI matrix across relevant OS/toolchains. Cache dependency/compile artifacts (e.g., ccache/sccache for C/C++). Publish build artifacts for review. ([ccache.dev][9])

---

## Documentation Discipline

Update `docs/review.md`, `docs/plan.md`, task files, and ADRs in the same change as code. Use MADR or a comparable ADR format for non-reversible choices. ([GitHub][3])

---

## Change Management

Small, single-concern changes. Conventional Commits; link task IDs; include repro steps, risks, and evidence. ([conventionalcommits.org][10])

---

## Security

No network writes without explicit directive. Validate external inputs. Enforce secret scanning in pre-commit/CI. Keep runtime containers minimal; prefer non-root execution. ([Docker Documentation][8])

---

## Memory Bank

`.agents/memory_bank.md` stores durable conventions:

* Code style and lint rules
* Project topology and entry points
* Canonical commands
* Domain invariants (security/perf budgets)
* Stack versions and lockfile policy

Keep high-signal, low-churn. Update when conventions change.

---

## Directives

Agents execute fenced `DIRECTIVE` blocks verbatim.

```DIRECTIVE
id: <ID>
goal: <single, testable outcome>
scope: <paths/**>
constraints:
  - <budgets, compatibility, security, exception-safety>
steps:
  - <ordered, deterministic actions>
verification:
  - <commands/checks for acceptance>
artifacts:
  - <docs/decisions/NNNN-*.md or other outputs>
logging:
  - append .agents/action_log.md with start/end + metrics
```

---

## Task Files

`tasks/plan_<slug>/<NN>_<task_name>.md`

```md
# <NN> <Task Name>

## Intent
Short rationale and expected impact.

## Scope
Paths/modules/configs to touch. Out-of-scope listed.

## Acceptance Criteria
- [ ] Functional outcomes
- [ ] Performance budget
- [ ] Security constraints
- [ ] Docs/tests updated

## Plan
Bullet micro-steps.

## Tests
New/updated tests and locations.

## Risks & Rollback
Known risks, kill-switch, revert plan.

## Evidence
Change link, CI run, logs, perf numbers.

## Status
Owner: <name/agent> • State: Todo | Doing | Review | Done • Updated: <ISO8601>
```

`tasks/status.md`

```md
# Status Board

## Todo
- [01] <Task> — link

## Doing
- [02] <Task> — link

## Review
- [03] <Task> — link

## Done
- [00] Bootstrap — link
```

```

Sources: Conventional Commits; MADR/ADR; Docker multi-stage builds; Sourcegraph Cody; Claude Code updates; ripgrep; fzf; universal-ctags; ccache. :contentReference[oaicite:14]{index=14}
::contentReference[oaicite:15]{index=15}
```

[1]: https://docs.sourcegraph.com/cody/overview?ref=bm&utm_source=chatgpt.com "Cody"
[2]: https://sourcegraph.com/code-search?utm_source=chatgpt.com "Code Search"
[3]: https://github.com/adr/madr?utm_source=chatgpt.com "adr/madr: Markdown Architectural Decision Records"
[4]: https://www.businessinsider.com/anthropic-ai-model-claude-sonnet-extend-coding-lead-2025-9?utm_source=chatgpt.com "Anthropic unveils latest AI model, aiming to extend its lead in coding intelligence"
[5]: https://github.com/BurntSushi/ripgrep?utm_source=chatgpt.com "ripgrep recursively searches directories for a regex pattern ..."
[6]: https://github.com/junegunn/fzf?utm_source=chatgpt.com "junegunn/fzf: :cherry_blossom: A command-line fuzzy finder"
[7]: https://github.com/universal-ctags/ctags?utm_source=chatgpt.com "universal-ctags/ctags: A maintained ctags implementation"
[8]: https://docs.docker.com/build/building/multi-stage/?utm_source=chatgpt.com "Multi-stage builds"
[9]: https://ccache.dev/manual/4.12.html?utm_source=chatgpt.com "ccache manual"
[10]: https://www.conventionalcommits.org/en/v1.0.0/?utm_source=chatgpt.com "Conventional Commits"
