# Repository instructions for Codex

## Project state

This is a PlatformIO firmware project for M5Stack Core2. `platformio.ini` defines the `m5stack-core2` environment with the Arduino framework. `src/main.cpp` is still the generated sample; there is no implemented meter behavior or test suite. Do not infer product requirements from the repository name.

## Working in this repository

- Read `README.md` for the current status. Use `docs/requirements.md` when deciding product behavior and `docs/development.md` for build and hardware workflow.
- Keep changes scoped to the user's request. When behavior is unspecified, record the assumption or ask for the missing requirement before implementing that behavior.
- Preserve the PlatformIO environment unless a requested feature requires a configuration change. Add dependencies only when code uses them.
- Put application code in `src/`, shared headers in `include/`, project libraries in `lib/`, and tests in `test/`. Update the relevant documentation when behavior or setup changes.
- Run `pio run -e m5stack-core2` after firmware or build configuration changes when PlatformIO is available. Report the actual result; do not claim a build passed if it could not run.
- Add focused tests for behavior that can be verified without hardware. Hardware dependent behavior needs an explicit manual check recorded with the change.
- Inspect the working tree before editing and preserve unrelated changes. Review the diff and report verification and remaining limitations at the end.
- Upload to a physical device only when the task calls for it and the correct device and port are known. Treat hardware tests as separate from compilation.

## Current product boundary

The product goal, input data source, display layout, controls, and success criteria are not yet specified. Capture decisions in `docs/requirements.md` as they are agreed; avoid presenting guesses as established requirements.
