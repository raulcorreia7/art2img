# art2img Makefile - CMake Wrapper ------------------------------------------------
# ------------------------------------------------------------------------------
# This Makefile is a convenience wrapper around the CMake build system.
# For advanced configuration, use CMake directly.
# ------------------------------------------------------------------------------

PROJECT := art2img
SHELL := /bin/bash
MAKEFLAGS += --warn-undefined-variables
MAKEFLAGS += --no-builtin-rules

# Detect number of processors for parallel compilation
NPROCS := $(shell nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
BUILDDIR := build

# Default goal
.DEFAULT_GOAL := all

# Help target
.PHONY: help
help:
	@echo "$(PROJECT) build system"
	@echo "======================="
	@echo ""
	@echo "This is a wrapper Makefile around CMake. For direct CMake usage:"
	@echo "  mkdir build && cd build && cmake .. && cmake --build ."
	@echo ""
	@echo "Available targets:"
	@printf "  %-20s%s\\n" "all" "Build all targets (default)"
	@printf "  %-20s%s\\n" "clean" "Clean build directory"
	@printf "  %-20s%s\\n" "test" "Run tests"
	@printf "  %-20s%s\\n" "install" "Install to system"
	@printf "  %-20s%s\\n" "help" "Show this help"
	@echo ""
	@echo "Advanced targets:"
	@printf "  %-20s%s\\n" "debug" "Build with debug configuration"
	@printf "  %-20s%s\\n" "release" "Build with release configuration"
	@printf "  %-20s%s\\n" "verbose" "Build with verbose output"
	@echo ""

# Ensure build directory exists
$(BUILDDIR):
	@mkdir -p $(BUILDDIR)

# Configure CMake if not already configured
$(BUILDDIR)/Makefile: $(BUILDDIR)
	@cd $(BUILDDIR) && cmake ..

# Main targets
.PHONY: all
all: $(BUILDDIR)/Makefile
	@echo "Building $(PROJECT) with CMake..."
	@cd $(BUILDDIR) && cmake --build . -- -j$(NPROCS)
	@echo "Build complete!"

.PHONY: debug
debug: $(BUILDDIR)
	@echo "Configuring debug build..."
	@cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=Debug ..
	@$(MAKE) all

.PHONY: release
release: $(BUILDDIR)
	@echo "Configuring release build..."
	@cd $(BUILDDIR) && cmake -DCMAKE_BUILD_TYPE=Release ..
	@$(MAKE) all

.PHONY: verbose
verbose: $(BUILDDIR)
	@echo "Building with verbose output..."
	@cd $(BUILDDIR) && cmake --build . --verbose

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILDDIR)
	@echo "Clean complete"

.PHONY: test
test: all
	@echo "Running tests..."
	@cd $(BUILDDIR) && ctest --output-on-failure
	@echo "Tests completed"

.PHONY: install
install: all
	@echo "Installing $(PROJECT)..."
	@cd $(BUILDDIR) && cmake --install .
	@echo "Installation complete"

# Legacy compatibility targets
.PHONY: linux windows windows-build verify verify-version procs
linux: all
windows: all
windows-build: all
verify: test
verify-version: all
procs:
	@echo "Detected $(NPROCS) processors"

# Convenience shortcuts
.PHONY: check
check: test

.PHONY: build
build: all

# Prevent creation of files from these targets
.PHONY: all debug release clean test install help linux windows windows-build verify verify-version procs check build verbose