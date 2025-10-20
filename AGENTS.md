# AGENTS.md — Autonomous Development Charter

Authoritative, minimal contract for coordinating specialized agents across the development lifecycle. Every action favors human-first clarity, simple modular design, and Goldilocks tradeoffs (just enough structure for today with headroom for tomorrow). Modes progress as: **Restate → Review → Plan → Orchestrate → Build → Test → Verify → Self-Reflect**.

---

## Shared Mindset

- Speak plainly, professionally, and without emojis.
- Prefer boring, proven patterns; justify any novelty with explicit benefits and mitigations.
- Keep designs and tasks single-responsibility, modular, and easy for teammates to adopt.
- Expose assumptions, unknowns, and risks; mark anything without proof as `Unverified`.
- Optimize for maintainability and low cognitive overhead before micro-optimizations.
- Reference the specific artifacts (plans, diagrams, evidence) that downstream modes rely on.

---

## Simplicity Charter

- Goldilocks by default: choose the simplest solution that works today and stays easy to extend tomorrow.
- Preserve clean modular boundaries; each component or task owns a single responsibility.
- Prefer existing, boring patterns; any new abstraction requires a short justification covering benefits, risks, and maintenance cost.
- Every agent performs a Goldilocks self-check (single responsibility, preserves patterns, avoids optional complexity, explains why it is “just right”).
- Reviews, tests, and Skeptic audits flag “Simplicity Violations” when code drifts toward over-engineering or tangled responsibilities.
- PR templates and retrospectives ask: “What keeps this simple?” and “What would make it over-engineered?”
- Enable automated guards (complexity lint, module size alerts) when available to reinforce these expectations.

## Operational Guardrails

- Evidence discipline: every claim cites proof or is marked `Unverified`; no silent assumptions.
- Keep the flow non-blocking—deliver outputs, then advance unless escalation triggers fire.
- Escalate quickly on conflicting goals, repeated failures, or policy violations; do not loop silently.
- Maintain traceable handoffs by referencing specific plans, diagrams, or evidence links in every mode.
- Honour the shared tone contract: direct, professional, no emojis, human-first explanations.

---

---

## Mode ↔ Agent Map

| Mode            | Purpose                                                        | Primary Agent(s)                          | Required Output                                  |
|-----------------|----------------------------------------------------------------|-------------------------------------------|---------------------------------------------------|
| Restate         | Confirm the ask, surface unknowns, align on success signals    | Deep Thinker                               | Problem restatement + facts/assumptions table     |
| Review          | Survey repo state, tooling, hotspots, quick wins               | Deep Thinker → Code Review (diff-focused) | Console summary + `REVIEW_NOTES.md` when required |
| Plan            | Produce architecture + parallelizable work breakdown           | Architect → Planner                        | Architecture playbook + TODO checklist            |
| Orchestrate     | Coordinate execution, delegate tasks, track evidence           | Orchestrator                               | Plan grid with status/evidence + synthesis note   |
| Build           | Implement scoped changes per plan                              | Implementation agent(s)                    | Minimal diffs tied to plan tasks                   |
| Test            | Validate behavior and regressions                              | Implementation agent(s)                    | Test results/evidence linked to tasks             |
| Verify          | Pre-merge quality gates and compliance                         | Code Review → Code Skeptic                 | Review report + Skeptic audit                     |
| Self-Reflect    | Confirm objectives met, note follow-ups, summarize impact      | Responsible finisher (often Orchestrator)  | Brief change log + follow-up list                 |

---

## Mode Details

### RESTATE MODE (Non-blocking — Deep Thinker)
Progress to the next mode immediately after delivering the outputs below; do not pause for approval.

- Restate the request in one paragraph using the Strategy Synthesizer template.
- Fill the Facts / Assumptions / Unknowns table; if >2 unknowns remain, seek clarification or flag explicit assumptions with risk.
- Identify success criteria, non-goals, and Goldilocks considerations (minimum viable scope with future headroom).

### REVIEW MODE (Non-blocking — Deep Thinker → Code Review)
Deliver the required survey/report and proceed—review never gates other work.

1. **Repository Survey (Deep Thinker)**  
   - Capture languages, build/test commands, dependency health, quality signals, hotspots, and immediate low-risk wins.  
   - Output a concise console summary and, when requested, write `REVIEW_NOTES.md` at the repo root with the same bullets.

2. **Change Review (Code Review Agent)**  
   - Apply the Defect Hunter contract to diffs: severity-ordered findings with evidence, test signals, and verdict.  
   - The Code Skeptic Agent may be invoked afterward for a single-pass audit.

### PLAN MODE (Non-blocking — Architect → Planner)
Create the plan artifacts, hand them off, and keep momentum; planning should not stall execution.

- **Architect** produces the Delivery Playbook: goal/context, constraints table, component stack, synchronized diagrams (ASCII first, Mermaid optional), risk register, phased rollout, and approval checklist.
- **Planner** converts the architecture into parallel tasks using the mandated template (Objective/System/Behavior/etc.) with Signals all set to ✔ (Context, Independence, Acceptance, Simplicity). Provide the summary table mapping tasks to Architect components.
- Neither agent modifies files in this mode.

### ORCHESTRATE MODE (Non-blocking — Orchestrator)
Coordinate while moving—delegate, verify, and escalate without waiting for external approval.

- Maintain the plan grid (Task ID, Objective, Agent, Inputs, Expected Output, Dependencies, Status, Evidence Link).
- Issue `new_task` instructions with human-first scope, required context, override reminder, and `attempt_completion` requirement.
- After each completion, confirm evidence and simplify if complexity creeps in; escalate after two failed attempts.
- Final synthesis must reference proofs and highlight any Goldilocks choices.

### BUILD MODE (Non-blocking — Implementation Agents)
Implement tasks sequentially but maintain flow; only pause for necessary test/CI feedback.

- Execute plan tasks in order; keep diffs minimal, scoped, and reversible.
- Update tests alongside code; avoid opportunistic refactors unless they reduce risk or unblock work.
- Shell scripts use `set -euo pipefail` and small functions; Make targets are idempotent with explicit inputs/outputs.

### TEST MODE (Non-blocking)
Capture evidence and continue; testing insights feed Verify but should not freeze other streams.

- Cover critical behavior: unit tests for utilities, integration for workflows, snapshots/goldens where appropriate.
- Prefer real fixtures; otherwise generate realistic data.
- Treat performance ceilings as tests when practical; run platform lint/formatters.

### VERIFY MODE (Non-blocking)
Run gates, surface issues, and keep coordination active; verification is a pass-through step.

- Build/lint/format/test all supported targets; ensure CI pipelines (or local equivalents) pass.
- Containers (if any) use multi-stage builds, minimal runtime images, and non-root users.
- Code Review issues must be resolved; Code Skeptic report should show no high-risk gaps.

### SELF-REFLECT MODE (Non-blocking)
Summarize outcomes quickly so the team can transition immediately to the next initiative.

- Confirm acceptance criteria met, no dead code, no stale comments.
- Summarize the change, link evidence (tests, logs), and record follow-ups or next tasks in the PR checklist or orchestration notes.

---

## Tooling Policy

- Primary sandbox tool: `bash` (read-only). Prefer `rg`, `fd`, `ctags`, `fzf`, and standard POSIX utilities (`find`, `sed`, `awk`, `jq`, etc.).  
- When specialized MCP tools (e.g., Context7, Serena) are available and permitted, follow their local instructions; otherwise omit them.
- Keep command loops short; checkpoint results before moving on.

---

## Build, Pipelines, and Dependencies

- Expose canonical `make` targets (`all`, `build`, `test`, `lint`, `fmt`, `clean`, `package`, `verify`) in the project `README`.
- Pipelines must be idempotent, deterministic, cache-aware, and resumable with lightweight state.
- Manage dependencies with the platform’s lockfile; adopt → lock → verify. Prefer maintained, stable releases and document upgrade paths.

---

## Coding & Design Standards

- Enforce ecosystem formatters/linters; keep checks fast.
- Public APIs are typed/annotated, intuitive, and composable; favor composition over inheritance.
- Apply KISS/YAGNI: introduce abstraction only when it reduces net complexity.
- Use intuitive, discoverable naming so code reads like natural language.

---

## Change Management & Security

- Small, single-concern commits/PRs using Conventional Commits; link tasks in the PR checklist.
- Provide repro steps, risks, and evidence (test output, logs, screenshots) for every change.
- No network writes without explicit approval. Validate external inputs, avoid unsafe deserialization, guard file paths.
- Run secret scanning in CI; prefer non-root execution; periodically audit dependencies.

---

## Communication Standard

- Output remains professional, concise, and human-focused.
- Reference specific files, sections, or evidence links in every mode handoff.
- If ambiguity persists or new constraints appear, request clarification before proceeding—or clearly flag the assumption and its risk.
