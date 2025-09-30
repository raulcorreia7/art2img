# AGENTS.md - art2image C++ Project

## Build Commands (Simplified)
- `make linux` - Build Linux binaries (art2image + art_diagnostic)
- `make windows` - Build Windows binaries (cross-compile from Linux)
- `make all-platforms` - Build for Linux + Windows
- `make clean` - Remove build artifacts
- `make test` - Test Linux binaries

## Docker Build
```bash
docker build -t art2image .
docker run --rm -v $(pwd)/bin:/build/bin art2image
```

## Code Style Guidelines
- **Language**: C++17 standard
- **Formatting**: 4-space indentation, Unix line endings
- **Naming**: snake_case for variables/functions, PascalCase for classes
- **Headers**: Use `#pragma once`, include guards not needed
- **Includes**: Group standard library, then project headers
- **Error Handling**: Use exceptions for fatal errors, return bool for recoverable errors
- **Memory**: Prefer RAII, avoid raw pointers, use std::vector for dynamic arrays

## Project Structure
- `src/` - Implementation files (.cpp)
- `include/` - Header files (.hpp)
- `bin/` - Compiled binaries
- `obj/` - Object files
- `tests/` - Test scripts and assets
- `vendor/` - Third-party dependencies (stb_image_write)

## Key Conventions
- Use namespaces (`art2image`)
- Classes are non-copyable, movable by default
- Header files contain declarations only
- Implementation files include corresponding headers first
- Error messages go to std::cerr, success to std::cout
- Thread-safe design with pthread support

## Environment Configuration
- Create `.env` from `.env.example` for custom paths
- Supports PALETTE_PATH, ART_FILES_DIR, OUTPUT_FORMAT, THREADS variables
- Use `make process-all` or `make process-single` with environment config

## Alpha Handling Features (New)
- **RGBA Output**: PNG files now include alpha channel for transparency
- **Magenta Keying**: Automatic alpha detection using magenta pixels (r8≥250, b8≥250, g8≤5)
- **Premultiplication**: Applied by default for proper upscaling (erases hidden RGB + premultiplies edges)
- **Matte Hygiene**: Optional alpha processing (erode + blur) for clean edges
- **Configuration**: Use `PngWriter::Options` to control alpha behavior

## Development Best Practices
- **Code Review**: Always run `make test-all` before committing changes
- **Documentation**: Update README.md when adding features or changing behavior
- **Testing**: Add new tests for new functionality in `tests/test_png.sh`
- **Error Handling**: Validate all inputs, provide clear error messages
- **Memory Safety**: Use RAII patterns, avoid manual memory management
- **Cross-Platform**: Test on both Windows (MSYS2) and Linux environments
- **Performance**: Profile with `make debug` before optimizing critical paths

## Agent Guidelines
- **Before Changes**: Run `make clean && make test-all` to ensure clean state
- **After Changes**: Verify with `make test` and check for memory leaks
- **Documentation**: Update AGENTS.md when discovering new patterns, conventions. Keep documentation always up to date.
- **Testing**: Always test with real ART files from `tests/assets/`
- **Error Cases**: Test edge cases (empty files, invalid palettes, corrupt data)
