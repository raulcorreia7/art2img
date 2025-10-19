# Pipeline Notes

## Summary
- Added dedicated `make fmt` and `make fmt-check` targets that delegate to the CMake `clang-format` tooling to keep the file lists consistent.
- Added a `make lint` target that runs `clang-tidy` (or emits a clear failure when it is not available) against all translation units using the generated `compile_commands.json` database.
- Code formatting and static analysis are now integrated into CI checks.

## Usage
```bash
make fmt         # Format sources in-place via clang-format
make fmt-check   # Verify formatting without modifying files
make lint        # Run clang-tidy analysis (requires clang-tidy or run-clang-tidy)
make verify      # Run comprehensive checks (build + test + lint + fmt-check)
```

Each target reconfigures the build directory with the standard project flags if necessary so that CMake tooling targets are available.

## Future Enhancements
- Provide a repository-wide `.clang-tidy` configuration to pin the desired check set and suppression policy.
- Cache clang-tidy results (e.g., with `llvm-tidy-cache`) to speed up repeated invocations on large ART datasets.
- Extend linting to shell scripts (via `shellcheck`) and Python utilities if/when they are introduced into the pipeline.
- Implement pre-commit hooks for automatic formatting and linting.
