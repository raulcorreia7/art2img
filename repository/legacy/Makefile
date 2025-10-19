# Makefile for art2img - Simplified and focused
# Supports Linux development and Windows cross-compilation
# Delegates to platform-specific build scripts

# Configuration
BUILD_DIR ?= build
JOBS ?= $(shell nproc)
VERSION ?= $(shell cmake -P cmake/print_version.cmake)

# Run integration tests for different platforms
test-intg: build
	@BUILD_TYPE=linux ./scripts/run_bats_tests.sh

test-intg-windows: mingw-windows
	@BUILD_TYPE=mingw-windows ./scripts/run_bats_tests.sh

test-intg-windows-x86: mingw-windows-x86
	@BUILD_TYPE=mingw-windows-x86 ./scripts/run_bats_tests.sh

test-intg-release: linux-x64-release
	@BUILD_TYPE=linux-x64-release ./scripts/run_bats_tests.sh

# Main targets
.PHONY: all build test clean install format fmt fmt-check lint help
.PHONY: mingw-windows mingw-windows-x86 test-windows doctor test-bats
.PHONY: linux-x64-release windows-x64-release windows-x86-release

# Default target - build for Linux
all: build

# Build for Linux using dedicated script
build:
	@./scripts/build/native/linux.sh --build-dir $(BUILD_DIR) -j $(JOBS)

# Build for Windows x64 (cross-compilation from Linux using MinGW) using dedicated script
mingw-windows:
	@./scripts/build/cross/mingw-windows64.sh --build-dir $(BUILD_DIR) -j $(JOBS)

# Build for Windows x86 (cross-compilation from Linux using MinGW) using dedicated script
mingw-windows-x86:
	@./scripts/build/cross/mingw-windows32.sh --build-dir $(BUILD_DIR) -j $(JOBS)

# Release builds using the dedicated scripts
linux-x64-release:
	@./scripts/build/native/linux.sh --build-dir $(BUILD_DIR)/linux-x64-release --build-type Release --use-direct-path -j $(JOBS)
	@cd $(BUILD_DIR)/linux-x64-release && ctest --output-on-failure

mingw-windows-x64-release:
	@./scripts/build/cross/mingw-windows64.sh --build-dir $(BUILD_DIR)/mingw-windows-x64-release --build-type Release -j $(JOBS)

mingw-windows-x86-release:
	@./scripts/build/cross/mingw-windows32.sh --build-dir $(BUILD_DIR)/mingw-windows-x86-release --build-type Release -j $(JOBS)

# Run tests on Linux
test: build
	@cd $(BUILD_DIR)/linux-x64 && ctest --output-on-failure

# Test Windows build (requires Wine)
test-windows: mingw-windows
	@./scripts/test_windows.sh $(BUILD_DIR)/mingw-windows-x64

test-windows-x86: mingw-windows-x86
	@./scripts/test_windows.sh $(BUILD_DIR)/mingw-windows-x86

# Install to system (from Linux build)
install: build
	@cd $(BUILD_DIR)/linux-x64 && cmake --install . --prefix /usr/local

# Code formatting using the Linux build
fmt:
	@./scripts/build/native/linux.sh --build-dir $(BUILD_DIR) --build-type Release
	@cd $(BUILD_DIR)/linux-x64 && cmake --build . --target clang-format

format: fmt

fmt-check:
	@./scripts/build/native/linux.sh --build-dir $(BUILD_DIR) --build-type Release
	@cd $(BUILD_DIR)/linux-x64 && cmake --build . --target clang-format-dry-run

# Check code formatting (dry run)
format-check: fmt-check

lint:
	@./scripts/build/native/linux.sh --build-dir $(BUILD_DIR) --build-type Release
	@cd $(BUILD_DIR)/linux-x64 && cmake --build . --target clang-tidy

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Help
help:
	@echo "art2img - Build Commands"
	@echo "  make all          - Build for Linux (default)"
	@echo "  make build        - Build for Linux"
	@echo "  make mingw-windows      - Cross-compile for Windows x64 using MinGW"
	@echo "  make mingw-windows-x86  - Cross-compile for Windows x86 using MinGW"
	@echo "  make linux-x64-release       - Build + test release configuration for Linux x64"
	@echo "  make mingw-windows-x64-release     - Build release configuration for Windows x64 using MinGW"
	@echo "  make mingw-windows-x86-release - Build release configuration for Windows x86 using MinGW"
	@echo "  make test         - Run tests on Linux"
	@echo "  make test-intg         - Run integration tests for Linux"
	@echo "  make test-intg-windows - Run integration tests for Windows (cross-compiled)"
	@echo "  make test-intg-windows-x86 - Run integration tests for Windows x86 (cross-compiled)"
	@echo "  make test-intg-release   - Run integration tests for Linux release build"
	@echo "  make test-windows - Test Windows build (requires Wine)"
	@echo "  make install      - Install to system"
	@echo "  make clean        - Remove build directory"
	@echo "  make fmt          - Format source code"
	@echo "  make fmt-check    - Check source code formatting"
	@echo "  make lint         - Run clang-tidy analysis"
	@echo "  make doctor       - Check system dependencies"

# Doctor script - verify system dependencies
doctor:
	@./scripts/doctor.sh
