# Contributing to art2img

Converts Build Engine ART files to PNG, TGA or BMP. C++17, CMake, tests under
`tests/` run by CTest.

## Requirements

- A C++17 compiler and CMake (`project()` sets `CMAKE_CXX_STANDARD 17`).
  CI installs `build-essential cmake ninja-build`; any generator works.
- `clang-format` and `clang-tidy` for the style and lint targets.

## Build

```sh
make build            # cmake -B build + parallel build (Release)
make debug            # same with BUILD_TYPE=Debug
BUILD_DIR=mybuild JOBS=4 make build   # override location and parallelism
```

Plain CMake works too: `cmake -B build && cmake --build build`.

## Test

```sh
make shareware        # download Duke3D shareware assets used by the tests
make test             # build, then ctest --output-on-failure
make test-unit        # ctest -L unit
make test-intg        # ctest -L integration
make test-e2e         # ctest -L e2e (needs make shareware)
```

## Style and lint

```sh
make fmt              # clang-format -i over include/ src/ cli/ tests/
make fmt-check        # fail on unformatted files
make lint             # clang-tidy with -Iinclude -Ivendor -std=c++17
```

`.clang-format` is the style authority; run `make fmt` before committing.

## Branch and pull request flow

1. Fork the repository and clone your fork.
2. Branch from `development` (the default branch): `git switch -c my-change`.
3. Build, run `make test`, `make fmt-check` and `make lint`, push, and open a
   pull request against `development`.

CI runs the build and CTest on Ubuntu and macOS, plus a CodeQL security
analysis, on every pull request. Green checks are required to merge.

## Before you open a pull request

- One conversion or fix per pull request; link the issue if there is one.
- Add or extend a test for behaviour changes; new formats need an e2e case.
- Regenerate nothing under `vendor/`; it is third-party code.
- `make clean` removes the build tree; `make distclean` also removes
  downloaded test data and generated images.
