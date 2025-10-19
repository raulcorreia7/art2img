# art2img

A C++23 library and tool for extracting and converting ART format assets (from Build engine games) to modern image formats.

## Status: 🚧 Architecture Refactor in Progress

This repository is currently undergoing a major architectural refactor as described in [`plan/tasks.md`](plan/tasks.md). The existing implementation has been moved to [`repository/legacy`](repository/legacy) for reference.

### New Architecture

The new C++23 architecture will feature:
- Stateless functions with plain structs
- `std::expected<T, Error>` error handling throughout
- Modern CMake build system
- Comprehensive test suite

### Directory Structure

```
├── include/art2img/     # New public headers (C++23)
├── src/                 # New implementation files
├── tests/               # New test suite
├── cmake/               # Build configuration
├── plan/                # Architecture and task documentation
├── docs/                # Legacy documentation
└── repository/legacy/   # Previous implementation (preserved)
```

### Building (New Architecture)

```bash
cmake -S . -B build -DART2IMG_ENABLE_LEGACY=ON
cmake --build build
```

### Legacy Implementation

The previous implementation remains available in `repository/legacy/` and can be built with:

```bash
cd repository/legacy
mkdir build && cd build
cmake ..
make
```

See [`repository/legacy/BUILDING.md`](repository/legacy/BUILDING.md) for detailed legacy build instructions.

## Documentation

- [Architecture Overview](plan/architecture.md)
- [Implementation Roadmap](plan/tasks.md)
- [Legacy Documentation](docs/)

## License

See [`repository/legacy/LICENSE`](repository/legacy/LICENSE) for license information.