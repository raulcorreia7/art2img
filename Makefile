# Simple Makefile wrapper for CMake
# Provides common build targets for the art2img project

# Configuration
BUILD_DIR ?= build
BUILD_TYPE ?= Release
JOBS ?= $(shell nproc)

# Main targets
.PHONY: all build clean test install help fmt fmt-check lint
.PHONY: windows-x64-mingw windows-x86-mingw windows-mingw
.PHONY: windows-x64-native windows-x86-native windows-native windows
.PHONY: macos-x64-osxcross macos-arm64-osxcross macos-osxcross
.PHONY: macos-x64-native macos-arm64-native macos-native macos
.PHONY: all-platforms check-mingw check-osxcross

# Default target (native Linux build)
all: build

# Native Linux build (current - unchanged)
build:
	@cmake -B build/linux_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@cmake --build build/linux_x64 --parallel $(JOBS)

# Prerequisite checks
check-mingw:
	@which x86_64-w64-mingw32-gcc > /dev/null 2>&1 || \
		(echo "MinGW not found. Install with: sudo pacman -S mingw-w64-gcc"; exit 1)

check-osxcross:
	@which o64-clang > /dev/null 2>&1 || \
		(echo "osxcross not found. Install osxcross first"; exit 1)

# Windows cross-compilation targets
windows-x64-mingw: check-mingw
	@cmake -B build/windows_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/windows_x64.cmake
	@cmake --build build/windows_x64 --parallel $(JOBS)

windows-x86-mingw: check-mingw
	@cmake -B build/windows_x86 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/windows_x86.cmake
	@cmake --build build/windows_x86 --parallel $(JOBS)

# Windows native targets (for Windows users)
windows-x64-native:
	@cmake -B build/windows_x64 -G "Visual Studio 17 2022" -A x64
	@cmake --build build/windows_x64 --config Release

windows-x86-native:
	@cmake -B build/windows_x86 -G "Visual Studio 17 2022" -A Win32
	@cmake --build build/windows_x86 --config Release

# macOS cross-compilation targets
macos-x64-osxcross: check-osxcross
	@cmake -B build/macos_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/macos_x64.cmake
	@cmake --build build/macos_x64 --parallel $(JOBS)

macos-arm64-osxcross: check-osxcross
	@cmake -B build/macos_arm64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/macos_arm64.cmake
	@cmake --build build/macos_arm64 --parallel $(JOBS)

# macOS native targets (for macOS users)
macos-x64-native:
	@cmake -B build/macos_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@cmake --build build/macos_x64 --parallel $(JOBS)

macos-arm64-native:
	@cmake -B build/macos_arm64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_OSX_ARCHITECTURES=arm64
	@cmake --build build/macos_arm64 --parallel $(JOBS)

# Platform family targets
windows-mingw: windows-x64-mingw windows-x86-mingw
windows-native: windows-x64-native windows-x86-native
windows: windows-mingw

macos-osxcross: macos-x64-osxcross macos-arm64-osxcross
macos-native: macos-x64-native macos-arm64-native
macos: macos-osxcross

# Build everything
all-platforms: all windows macos

# Run tests (Linux native)
test: build
	@cd build/linux_x64 && ctest --output-on-failure

# Install the project
install: build
	@cd $(BUILD_DIR) && cmake --install .

# Clean build directory
clean:
	@rm -rf build/

# Format code using clang-format directly
fmt:
	@echo "Formatting source code..."
	@find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format -i
	@echo "Code formatted successfully"

# Check formatting without modifying files
fmt-check:
	@echo "Checking code formatting..."
	@find . -name '*.cpp' -o -name '*.hpp' | xargs clang-format --dry-run --Werror
	@echo "Code formatting check passed"

# Run linting using clang-tidy directly
lint:
	@echo "Running static analysis..."
	@cmake -B build/lint -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF
	@cd build/lint && run-clang-tidy -config '{}' -p . $(find .. -name '*.cpp' -o -name '*.hpp') || true
	@echo "Static analysis completed"

# Help
help:
	@echo "art2img - Cross-Platform CMake Build System"
	@echo ""
	@echo "Native Builds:"
	@echo "  make all                    - Build Linux x64 (default)"
	@echo "  make build                  - Configure and build Linux x64"
	@echo ""
	@echo "Windows Cross-Compilation (from Linux):"
	@echo "  make windows-x64-mingw      - Windows 64-bit cross-compile"
	@echo "  make windows-x86-mingw      - Windows 32-bit cross-compile"
	@echo "  make windows-mingw          - All Windows cross-compilation"
	@echo ""
	@echo "Windows Native Builds (from Windows):"
	@echo "  make windows-x64-native     - Native Windows 64-bit"
	@echo "  make windows-x86-native     - Native Windows 32-bit"
	@echo "  make windows-native         - All Windows native builds"
	@echo "  make windows                - All Windows builds (cross-compile)"
	@echo ""
	@echo "macOS Cross-Compilation (from Linux):"
	@echo "  make macos-x64-osxcross     - macOS Intel cross-compile"
	@echo "  make macos-arm64-osxcross   - macOS ARM64 cross-compile"
	@echo "  make macos-osxcross         - All macOS cross-compilation"
	@echo ""
	@echo "macOS Native Builds (from macOS):"
	@echo "  make macos-x64-native       - Native macOS Intel"
	@echo "  make macos-arm64-native     - Native macOS Apple Silicon"
	@echo "  make macos-native           - All macOS native builds"
	@echo "  make macos                  - All macOS builds (cross-compile)"
	@echo ""
	@echo "Platform Groups:"
	@echo "  make all-platforms          - Build all platforms"
	@echo ""
	@echo "Development:"
	@echo "  make test                   - Run tests (Linux)"
	@echo "  make install                - Install the project"
	@echo "  make clean                  - Remove all build directories"
	@echo "  make fmt                    - Format source code (in-place)"
	@echo "  make fmt-check              - Check formatting without modifying"
	@echo "  make lint                   - Run static analysis"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE=$(BUILD_TYPE)    - Build type (Debug/Release)"
	@echo "  JOBS=$(JOBS)               - Number of parallel jobs"
	@echo ""
	@echo "Build Directory Structure:"
	@echo "  build/linux_x64/           - Linux x64 binaries"
	@echo "  build/windows_x64/          - Windows x64 binaries"
	@echo "  build/windows_x86/          - Windows x86 binaries"
	@echo "  build/macos_x64/            - macOS Intel binaries"
	@echo "  build/macos_arm64/          - macOS ARM64 binaries"