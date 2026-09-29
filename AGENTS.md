# Repository instructions for Codex

## Project and source of truth

This is a PlatformIO firmware project for M5Stack Core2. `platformio.ini` defines the `m5stack-core2` environment with the Arduino framework. `test/test_environment` checks the board and USB serial path, not product behavior. Use `docs/requirements.md` for the agreed product behavior and check the current source and work log for implementation status.

- `README.md`: project entry point and current structure.
- `docs/requirements.md`: agreed product behavior and open product questions.
- `docs/development.md`: development environment, build, upload, and verification procedures.
- `docs/work-log.md`: chronological task progress, decisions, verification results, blockers, and handoff notes.

Use both conversation context and the relevant repository files when past information is needed. At the start of a task, inspect the working tree and the latest work-log entry; read other documents as the task requires. Check recorded facts against the code and Git history, and correct discrepancies in the appropriate file.

## Documentation practice

- Keep specifications, decisions, assumptions, progress, blockers, and verification results in repository files rather than only in conversation context. Update the relevant file during the task and before handing off unfinished work.
- Treat `README.md`, `AGENTS.md`, `docs/requirements.md`, and `docs/development.md` as coherent current documents. When changing one, review the whole document and its related documents; integrate the new information into the right sections and revise or remove stale and conflicting statements. Write the result as a unified document, without append-only wording or a visible patchwork of additions.
- Preserve the chronological record in `docs/work-log.md`. At meaningful milestones, record the purpose, changes, decisions and reasons, exact verification and results, blockers, and next action. If a later entry supersedes an earlier status or decision, say so explicitly; keep the earlier entry as history.
- Keep product requirements in `docs/requirements.md`, procedures in `docs/development.md`, and transient task status in `docs/work-log.md`. Avoid duplicate rules or facts that can drift apart.
- Do not commit personal information, credentials, tokens, device serial numbers, MAC addresses, or raw diagnostic logs that may contain them. Use placeholders for machine-specific ports and paths in documentation, and inspect the staged diff for sensitive data before each commit.

## Development and verification

- Keep changes scoped to the user's request. When behavior is unspecified, document a justified assumption or ask for the missing requirement before implementing it.
- Preserve the PlatformIO environment unless a requested feature requires a change. Add dependencies only when code uses them. Put application code in `src/`, shared headers in `include/`, project libraries in `lib/`, and tests in `test/`.
- Run `pio run -e m5stack-core2` after firmware or build configuration changes when PlatformIO is available. Add focused tests for behavior that can be verified without hardware. Report actual build, test, and hardware results separately; never claim a check passed if it could not run.
- Upload to a physical device only when the task calls for it and the correct device and port are known. Record the procedure and expected result for hardware-dependent checks.
- Review the diff and verify each reviewable unit. Commit small, coherent units frequently with their related documentation; do not bundle unrelated work or leave completed units uncommitted. Preserve unrelated working-tree changes and report remaining limitations at handoff.
