# Simple Makefile wrapper for CMake
# Provides common build targets for the art2img project

# Configuration
BUILD_DIR ?= build
BUILD_TYPE ?= Debug
JOBS ?= $(shell nproc)

# Main targets
.PHONY: all build clean test install help fmt lint

# Default target
all: build

# Configure and build the project
build:
	@cmake -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)

# Run tests
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure

# Install the project
install: build
	@cd $(BUILD_DIR) && cmake --install .

# Clean build directory
clean:
	@rm -rf $(BUILD_DIR)

# Format code (if clang-format target exists)
fmt: build
	@cd $(BUILD_DIR) && cmake --build . --target clang-format 2>/dev/null || echo "clang-format target not available"

# Run linting (if clang-tidy target exists)
lint: build
	@cd $(BUILD_DIR) && cmake --build . --target clang-tidy 2>/dev/null || echo "clang-tidy target not available"

# Help
help:
	@echo "art2img - Simple CMake Wrapper"
	@echo "  make all           - Build the project (default)"
	@echo "  make build         - Configure and build the project"
	@echo "  make test          - Run tests"
	@echo "  make install       - Install the project"
	@echo "  make clean         - Remove build directory"
	@echo "  make fmt           - Format source code"
	@echo "  make lint          - Run static analysis"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_DIR=$(BUILD_DIR)  - Build directory"
	@echo "  BUILD_TYPE=$(BUILD_TYPE) - Build type (Debug/Release)"
	@echo "  JOBS=$(JOBS)           - Number of parallel jobs"