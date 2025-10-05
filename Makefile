# Makefile for art2img - Simplified and focused
# Supports Linux development and Windows cross-compilation

# Configuration
BUILD_DIR ?= build
JOBS ?= $(shell nproc)
VERSION ?= $(shell cmake -P cmake/print_version.cmake)

WINDOWS_TOOLCHAIN_X64 := cmake/windows-toolchain.cmake
WINDOWS_TOOLCHAIN_X86 := cmake/windows-x86-toolchain.cmake

LINUX_RELEASE_DIR := $(BUILD_DIR)/linux-release
WINDOWS_RELEASE_DIR := $(BUILD_DIR)/windows-release
WINDOWS_X86_RELEASE_DIR := $(BUILD_DIR)/windows-x86-release

CMAKE_BASE_FLAGS := -DBUILD_TESTS=ON
CMAKE_RELEASE_FLAGS := $(CMAKE_BASE_FLAGS) -DCMAKE_BUILD_TYPE=Release -DBUILD_DIAGNOSTIC=ON -DBUILD_SHARED_LIBS=OFF

# Main targets
.PHONY: all build test clean install format fmt fmt-check lint help
.PHONY: windows windows-x86 test-windows doctor
.PHONY: linux-release windows-release windows-x86-release

# Default target - build for Linux
all: build

# Build for Linux
build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release $(CMAKE_BASE_FLAGS)
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

# Build for Windows x64 (cross-compilation)
windows:
	@mkdir -p $(BUILD_DIR)/windows
	@cd $(BUILD_DIR)/windows && cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../$(WINDOWS_TOOLCHAIN_X64) -DBUILD_SHARED_LIBS=OFF
	@cmake --build $(BUILD_DIR)/windows --parallel $(JOBS)

# Build for Windows x86 (cross-compilation)
windows-x86:
	@mkdir -p $(BUILD_DIR)/windows-x86
	@cd $(BUILD_DIR)/windows-x86 && cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../$(WINDOWS_TOOLCHAIN_X86) -DBUILD_SHARED_LIBS=OFF
	@cmake --build $(BUILD_DIR)/windows-x86 --parallel $(JOBS)

linux-release:
	@mkdir -p $(LINUX_RELEASE_DIR)
	@cd $(LINUX_RELEASE_DIR) && cmake ../.. $(CMAKE_RELEASE_FLAGS)
	@cmake --build $(LINUX_RELEASE_DIR) --parallel $(JOBS)
	@cd $(LINUX_RELEASE_DIR) && ctest --output-on-failure

windows-release:
	@mkdir -p $(WINDOWS_RELEASE_DIR)
	@cd $(WINDOWS_RELEASE_DIR) && cmake ../.. $(CMAKE_RELEASE_FLAGS) -DCMAKE_TOOLCHAIN_FILE=../../$(WINDOWS_TOOLCHAIN_X64)
	@cmake --build $(WINDOWS_RELEASE_DIR) --parallel $(JOBS)

windows-x86-release:
	@mkdir -p $(WINDOWS_X86_RELEASE_DIR)
	@cd $(WINDOWS_X86_RELEASE_DIR) && cmake ../.. $(CMAKE_RELEASE_FLAGS) -DCMAKE_TOOLCHAIN_FILE=../../$(WINDOWS_TOOLCHAIN_X86)
	@cmake --build $(WINDOWS_X86_RELEASE_DIR) --parallel $(JOBS)

# Run tests on Linux
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Test Windows build (requires Wine)
test-windows: windows
	@./scripts/test_windows.sh build/windows

test-windows-x86: windows-x86
	@./scripts/test_windows.sh build/windows-x86 build/windows-x86

# Install to system
install: build
	@cd $(BUILD_DIR) && cmake --install . --prefix /usr/local

# Code formatting
fmt:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release $(CMAKE_BASE_FLAGS)
	@cmake --build $(BUILD_DIR) --target clang-format

format: fmt

fmt-check:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release $(CMAKE_BASE_FLAGS)
	@cmake --build $(BUILD_DIR) --target clang-format-dry-run

# Check code formatting (dry run)
format-check: fmt-check

lint:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release $(CMAKE_BASE_FLAGS)
	@cmake --build $(BUILD_DIR) --target clang-tidy

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Help
help:
	@echo "art2img - Build Commands"
	@echo "  make all          - Build for Linux (default)"
	@echo "  make build        - Build for Linux"
	@echo "  make windows      - Cross-compile for Windows x64"
	@echo "  make windows-x86  - Cross-compile for Windows x86"
	@echo "  make linux-release       - Build + test release configuration for Linux"
	@echo "  make windows-release     - Build release configuration for Windows x64"
	@echo "  make windows-x86-release - Build release configuration for Windows x86"
	@echo "  make test         - Run tests on Linux"
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
