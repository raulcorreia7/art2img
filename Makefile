# Makefile for art2img - Simplified and focused
# Supports Linux development and Windows cross-compilation

# Configuration
BUILD_DIR ?= build
JOBS ?= $(shell nproc)

# Main targets
.PHONY: all build test clean install format help
.PHONY: windows test-windows

# Default target - build for Linux
all: build

# Build for Linux
build:
	@mkdir -p $(BUILD_DIR)
	@cd $(BUILD_DIR) && cmake .. -DCMAKE_BUILD_TYPE=Release
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

# Build for Windows (cross-compilation)
windows:
	@mkdir -p $(BUILD_DIR)/windows
	@cd $(BUILD_DIR)/windows && cmake ../.. -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=../../cmake/windows-toolchain.cmake -DBUILD_SHARED_LIBS=OFF
	@cmake --build $(BUILD_DIR)/windows --parallel $(JOBS)

# Run tests on Linux
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Test Windows build (requires Wine)
test-windows: windows
	@./scripts/test_windows.sh

# Install to system
install: build
	@cd $(BUILD_DIR) && cmake --install . --prefix /usr/local

# Code formatting
format:
	@./format_code.sh

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Help
help:
	@echo "art2img - Build Commands"
	@echo "  make all          - Build for Linux (default)"
	@echo "  make build        - Build for Linux"
	@echo "  make windows      - Cross-compile for Windows"
	@echo "  make test         - Run tests on Linux"
	@echo "  make test-windows - Test Windows build (requires Wine)"
	@echo "  make install      - Install to system"
	@echo "  make clean        - Remove build directory"
	@echo "  make format       - Format source code"