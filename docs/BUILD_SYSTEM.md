# art2img Build System Documentation

This document describes the modernized CMake build system for art2img, including usage examples, cross-compilation setup, and migration guide.

## Overview

The art2img build system has been modernized to provide:
- **Unified cross-compilation** with a single toolchain file
- **Modular CMake functions** for consistent build patterns
- **Centralized dependency management** with version pinning
- **Standardized testing setup** with automatic discovery
- **CMake presets** for common configurations
- **Enhanced developer experience** with better error messages

## Quick Start

### Native Builds

#### Linux (Native)
```bash
# Using CMake presets (recommended)
cmake --preset linux-x64
cmake --build --preset linux-x64

# Traditional CMake
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

#### Windows (Native)
```bash
# When building on Windows natively
cmake --preset windows-native
cmake --build --preset windows-native

# Or traditional CMake on Windows
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --parallel
```

#### macOS (Native)
```bash
# When building on macOS natively
cmake --preset macos-native
cmake --build --preset macos-native

# Or traditional CMake on macOS
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(sysctl -n hw.ncpu)
```

### Cross-Compilation (from Linux)

```bash
# Windows x64
cmake --preset windows-x64
cmake --build --preset windows-x64

# Windows x86
cmake --preset windows-x86
cmake --build --preset windows-x86

# macOS ARM64
cmake --preset macos-arm64
cmake --build --preset macos-arm64

# macOS Intel
cmake --preset macos-x64
cmake --build --preset macos-x64
```

### Testing

```bash
# Run all tests
cmake --build --preset linux-x64-debug
ctest --preset all

# Run only unit tests
ctest --preset unit

# Run only integration tests
ctest --preset integration
```

## Build Configuration

### CMake Presets

The project provides several CMake presets for common scenarios:

| Preset | Description | Use Case |
|--------|-------------|----------|
| `linux-x64` | Native Linux release build | Development, production |
| `linux-x64-debug` | Native Linux debug build | Development, debugging |
| `windows-native` | Native Windows build | Windows development (when on Windows) |
| `windows-x64` | Windows x64 cross-compile | Windows releases from Linux |
| `windows-x86` | Windows x86 cross-compile | Legacy Windows support |
| `macos-native` | Native macOS build | macOS development (when on macOS) |
| `macos-x64` | macOS Intel cross-compile | macOS releases from Linux |
| `macos-arm64` | macOS ARM64 cross-compile | Apple Silicon releases |
| `coverage` | Coverage-enabled build | Code coverage analysis |
| `benchmarks` | Benchmark-enabled build | Performance testing |

### Build Options

| Option | Default | Description |
|--------|---------|-------------|
| `BUILD_CLI` | `ON` | Build the CLI executable |
| `BUILD_TESTING` | `ON` | Build test suite |
| `BUILD_BENCHMARKS` | `OFF` | Build performance benchmarks |
| `STATIC_LINKING` | `ON` (cross-compile) | Enable static linking |
| `TARGET_PLATFORM` | `linux-x64` | Target platform for cross-compilation |

## Cross-Compilation

### Unified Toolchain System

The project uses a unified toolchain file (`cmake/Toolchain.cmake`) that supports multiple platforms:

```bash
# Usage
cmake -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain.cmake -DTARGET_PLATFORM=<platform> [options]

# Supported platforms
TARGET_PLATFORM=linux-x64      # Native Linux
TARGET_PLATFORM=windows-native # Native Windows (when on Windows)
TARGET_PLATFORM=windows-x64    # Windows 64-bit cross-compile
TARGET_PLATFORM=windows-x86    # Windows 32-bit cross-compile
TARGET_PLATFORM=macos-native   # Native macOS (when on macOS)
TARGET_PLATFORM=macos-x64      # macOS Intel cross-compile
TARGET_PLATFORM=macos-arm64    # macOS ARM64 cross-compile
```

### Prerequisites

#### Windows Cross-Compilation (Linux)
```bash
# Ubuntu/Debian
sudo apt-get install mingw-w64

# Verify installation
x86_64-w64-mingw32-gcc --version
i686-w64-mingw32-gcc --version
```

#### macOS Cross-Compilation (Linux)
```bash
# Install osxcross (follow osxcross documentation)
# Verify installation
o64-clang --version
oa64-clang --version
```

### Cross-Compilation Examples

```bash
# Windows x64 release (from Linux)
cmake -B build/windows-x64 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain.cmake \
    -DTARGET_PLATFORM=windows-x64 \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/windows-x64 --parallel

# macOS ARM64 release (from Linux)
cmake -B build/macos-arm64 \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain.cmake \
    -DTARGET_PLATFORM=macos-arm64 \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/macos-arm64 --parallel
```

### Native Build Examples

```bash
# Native Windows build (when on Windows)
cmake -B build/windows-native \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain.cmake \
    -DTARGET_PLATFORM=windows-native \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/windows-native --parallel

# Native macOS build (when on macOS)
cmake -B build/macos-native \
    -DCMAKE_TOOLCHAIN_FILE=cmake/Toolchain.cmake \
    -DTARGET_PLATFORM=macos-native \
    -DCMAKE_BUILD_TYPE=Release

cmake --build build/macos-native --parallel
```

## Dependency Management

### Centralized Version Control

All dependency versions are centralized in `cmake/Dependencies.cmake`:

```cmake
set(ART2IMG_STB_VERSION "fede005abaf93d9d7f3a679d1999b2db341b360f")
set(ART2IMG_DOCTEST_VERSION "2.4.12")
set(ART2IMG_CLI11_VERSION "2.5.0")
set(ART2IMG_FMT_VERSION "11.0.2")
set(ART2IMG_BENCHMARK_VERSION "1.8.3")
```

### Offline Support

The build system supports offline builds with CPM.cmake caching:

```bash
# Set cache location
export CPM_SOURCE_CACHE=$HOME/.cache/cpm

# First run (online) - downloads and caches dependencies
cmake --preset linux-x64

# Subsequent runs (offline) - uses cached dependencies
cmake --preset linux-x64
```

### Adding New Dependencies

1. Update version in `cmake/Dependencies.cmake`
2. Add dependency function if needed
3. Call dependency function in `setup_art2img_dependencies()`

```cmake
# Example: Add new dependency
set(ART2IMG_NEW_LIB_VERSION "1.2.3")

function(add_new_lib_dependency)
    CPMAddPackage(
        NAME new_lib
        VERSION ${ART2IMG_NEW_LIB_VERSION}
        GITHUB_REPOSITORY "user/new_lib"
        GIT_TAG "v${ART2IMG_NEW_LIB_VERSION}"
    )
    
    if(new_lib_ADDED)
        message(STATUS "Added new_lib dependency: ${new_lib_SOURCE_DIR}")
    endif()
endfunction()

# Add to setup_art2img_dependencies()
function(setup_art2img_dependencies)
    # ... existing dependencies ...
    add_new_lib_dependency()
    # ... rest of setup ...
endfunction()
```

## Testing System

### Test Discovery

The build system automatically discovers and categorizes tests:

- **Unit tests**: Fast, isolated tests in `tests/unit/`
- **Integration tests**: Slower tests with external dependencies in `tests/integration/`
- **Performance tests**: Benchmarking tests (when enabled)

### Test Organization

```
tests/
├── CMakeLists.txt          # Main test configuration
├── test_setup.cpp          # Common test setup
├── assets/                 # Test fixtures and data
│   ├── PALETTE.DAT
│   └── TILES000.ART
├── unit/                   # Unit tests
│   ├── core/
│   │   └── error/
│   │       └── test_error.cpp
│   └── io/
│       └── test_grp.cpp
└── integration/            # Integration tests
    └── test_full_workflow.cpp
```

### Adding Tests

#### Individual Test Files

```cpp
// tests/unit/core/test_my_feature.cpp
#include <doctest/doctest.h>
#include <art2img/core/my_feature.hpp>

TEST_CASE("My feature basic functionality") {
    // Test implementation
}
```

#### Test Suites

```cmake
# In tests/CMakeLists.txt or subdirectory
add_art2img_test_suite(
    NAME my_test_suite
    SOURCES test_my_feature.cpp
    DEPENDS art2img_core
    TYPE unit
    TIMEOUT 60
)
```

### Running Tests

```bash
# All tests
ctest --preset all

# Specific categories
ctest --preset unit          # Unit tests only
ctest --preset integration   # Integration tests only

# With verbose output
ctest --preset all --verbose

# Specific test
ctest --preset all -R "my_test"
```

## Modular CMake Functions

### Library Creation

```cmake
# Core library
add_art2img_core_library(
    NAME my_lib
    SOURCES my_lib.cpp
    HEADERS my_lib.hpp
    PUBLIC_DEPENDS fmt::fmt
    VERSION 1.0.0
)

# Interface library
add_art2img_library(
    NAME my_interface
    TYPE INTERFACE
    PUBLIC_INCLUDES include
    PUBLIC_DEPENDS some_lib
)
```

### Executable Creation

```cmake
# Standard executable
add_art2img_executable(
    NAME my_tool
    SOURCES main.cpp utils.cpp
    DEPENDS art2img_core fmt::fmt
    INSTALL_COMPONENT tools
)

# CLI executable (with standard installation)
add_art2img_cli_executable(
    NAME my_cli
    SOURCES cli_main.cpp
    DEPENDS art2img_core CLI11::CLI11
)
```

## Migration Guide

### From Old Build System

#### 1. Update Build Commands

**Old:**
```bash
make build
```

**New:**
```bash
cmake --preset linux-x64
cmake --build --preset linux-x64
```

#### 2. Update Cross-Compilation

**Old:**
```bash
make windows-x64-mingw
```

**New:**
```bash
cmake --preset windows-x64
cmake --build --preset windows-x64
```

#### 3. Update Testing

**Old:**
```bash
make test
```

**New:**
```bash
cmake --preset linux-x64-debug
ctest --preset all
```

### Updating CMakeLists.txt Files

#### Before (Old Pattern)
```cmake
add_executable(my_exe main.cpp)
target_link_libraries(my_exe PRIVATE art2img_core)
target_include_directories(my_exe PRIVATE include)
target_compile_options(my_exe PRIVATE -Wall -Wextra)
```

#### After (New Pattern)
```cmake
add_art2img_executable(
    NAME my_exe
    SOURCES main.cpp
    DEPENDS art2img_core
    INCLUDES include
)
```

## Troubleshooting

### Common Issues

#### Toolchain Not Found
```
Error: MinGW C compiler not found: x86_64-w64-mingw32-gcc
```

**Solution:** Install the required toolchain:
```bash
sudo apt-get install mingw-w64
```

#### CMake Presets Not Available
```
Error: Unknown preset "linux-x64"
```

**Solution:** Ensure you're using CMake 3.20+ and the CMakePresets.json file exists.

#### Dependency Download Fails
```
Error: Failed to download dependency
```

**Solution:** Check network connection or use offline cache:
```bash
export CPM_SOURCE_CACHE=$HOME/.cache/cpm
```

### Debug Mode

Enable verbose output for debugging:

```bash
# Verbose configuration
cmake --preset linux-x64 --debug-output

# Verbose build
cmake --build --preset linux-x64 --verbose

# Verbose testing
ctest --preset all --verbose
```

## Performance Optimization

### Build Performance

```bash
# Parallel builds
cmake --build --preset linux-x64 --parallel $(nproc)

# Use Ninja generator (faster builds)
cmake --preset linux-x64 -G Ninja
```

### Caching

```bash
# Use ccache for faster rebuilds
export CC="ccache gcc"
export CXX="ccache g++"

# Or configure in CMake
cmake --preset linux-x64 -DCMAKE_C_COMPILER_LAUNCHER=ccache -DCMAKE_CXX_COMPILER_LAUNCHER=ccache
```

## Contributing

When modifying the build system:

1. **Test all platforms** - Ensure changes work across all supported platforms
2. **Update documentation** - Keep this file and relevant comments updated
3. **Use modular functions** - Leverage the provided CMake functions
4. **Maintain backward compatibility** - Don't break existing workflows
5. **Test presets** - Verify CMake presets work correctly

### Build System Development

```bash
# Test build system changes
cmake --preset linux-x64-debug
cmake --build --preset linux-x64-debug
ctest --preset all

# Test cross-compilation
cmake --preset windows-x64
cmake --build --preset windows-x64

# Validate presets
cmake --list-presets
```

## Architecture Diagram

```
art2img Build System
├── CMakeLists.txt (root)
│   ├── cmake/Toolchain.cmake (unified cross-compilation)
│   ├── cmake/Dependencies.cmake (centralized deps)
│   ├── cmake/AddLibrary.cmake (library functions)
│   ├── cmake/AddExecutable.cmake (executable functions)
│   └── cmake/SetupTesting.cmake (test system)
├── CMakePresets.json (build configurations)
├── cli/CMakeLists.txt (CLI build)
└── tests/CMakeLists.txt (test configuration)
```

This modernized build system provides a solid foundation for development, testing, and distribution of the art2img project across multiple platforms.