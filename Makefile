# art2img - Build, test, and maintenance tasks
# Simple wrapper around CMake for convenience

.PHONY: all build test clean fmt fmt-check lint help install

# Configuration
BUILD_DIR ?= build
BUILD_TYPE ?= Release
JOBS ?= $(shell nproc 2>/dev/null || echo 4)
CMAKE_FLAGS = -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)

# Source files for formatting
SRC_FILES = $(shell find include src cli tests -name '*.cpp' -o -name '*.hpp' 2>/dev/null | grep -v tests/include/)
SRC_COUNT = $(words $(SRC_FILES))

# ============================================================================
# Default Target
# ============================================================================

all: build

# ============================================================================
# Build Targets
# ============================================================================

build:
	@mkdir -p $(BUILD_DIR)
	@cmake -B $(BUILD_DIR) $(CMAKE_FLAGS)
	@cmake --build $(BUILD_DIR) --parallel $(JOBS)
	@echo "Build complete: $(BUILD_DIR)/"

debug:
	@$(MAKE) build BUILD_TYPE=Debug

release:
	@$(MAKE) build BUILD_TYPE=Release

# ============================================================================
# Test Targets
# ============================================================================

test: build
	@rm -rf test_output
	@cd $(BUILD_DIR) && ctest --output-on-failure

test-verbose: build
	@rm -rf test_output
	@cd $(BUILD_DIR) && ctest --output-on-failure -V

test-unit: build
	@rm -rf test_output
	@cd $(BUILD_DIR) && ctest -L unit --output-on-failure

test-intg: build
	@rm -rf test_output
	@cd $(BUILD_DIR) && ctest -L integration --output-on-failure

test-smoke: build
	@rm -rf test_output
	@cd $(BUILD_DIR) && ctest -L e2e --output-on-failure

test-e2e: test-smoke

# ============================================================================
# Code Quality (Parallel)
# ============================================================================

fmt:
	@echo "Formatting $(SRC_COUNT) files with $(JOBS) jobs..."
	@echo $(SRC_FILES) | tr ' ' '\n' | xargs -P$(JOBS) -I{} clang-format -i {}
	@echo "Done"

fmt-check:
	@echo "Checking format of $(SRC_COUNT) files with $(JOBS) jobs..."
	@echo $(SRC_FILES) | tr ' ' '\n' | xargs -P$(JOBS) -I{} \
		sh -c 'clang-format --dry-run --Werror "{}" 2>&1 | grep -q error && echo "FAIL: {}" && exit 1 || true' || \
		(echo "Formatting check failed. Run 'make fmt' to fix." && exit 1)
	@echo "Formatting OK"

lint:
	@echo "Running clang-tidy on $(SRC_COUNT) files with $(JOBS) jobs..."
	@echo $(SRC_FILES) | tr ' ' '\n' | xargs -P$(JOBS) -I{} \
		sh -c 'clang-tidy "{}" -- -Iinclude -Ivendor -std=c++17 2>&1 | grep -E "(warning|error):" | head -5 | sed "s|^|{}: |"' | head -50 || \
		echo "No issues found"

# ============================================================================
# Maintenance
# ============================================================================

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Cleaned build directory"

distclean: clean
	@rm -rf tests/shareware/*.zip
	@rm -rf test_output
	@find . -name "*.png" -not -path "./test_output/*" -delete 2>/dev/null || true
	@find . -name "*.tga" -not -path "./test_output/*" -delete 2>/dev/null || true
	@find . -name "*.bmp" -not -path "./test_output/*" -delete 2>/dev/null || true
	@echo "Cleaned all generated files"

install: build
	@cmake --install $(BUILD_DIR) --prefix $(DESTDIR)/usr/local

# ============================================================================
# Shareware Test Data
# ============================================================================

shareware:
	@./scripts/download_shareware.sh

# ============================================================================
# Help
# ============================================================================

help:
	@echo "art2img - Build System"
	@echo ""
	@echo "Build Targets:"
	@echo "  make build        - Build the project (Release, uses all CPUs)"
	@echo "  make debug        - Build with Debug symbols"
	@echo "  make release      - Build optimized Release"
	@echo ""
	@echo "Test Targets:"
	@echo "  make test         - Run all tests"
	@echo "  make test-verbose - Run tests with verbose output"
	@echo "  make test-unit    - Run unit tests only"
	@echo "  make test-intg    - Run integration tests only"
	@echo "  make test-e2e     - Run end-to-end tests only"
	@echo ""
	@echo "Code Quality (parallel with xargs):"
	@echo "  make fmt          - Format all source files"
	@echo "  make fmt-check    - Check formatting without modifying"
	@echo "  make lint         - Run clang-tidy linter"
	@echo ""
	@echo "Maintenance:"
	@echo "  make clean        - Remove build directory"
	@echo "  make distclean    - Remove all generated files"
	@echo "  make install      - Install to system"
	@echo ""
	@echo "Test Data:"
	@echo "  make shareware    - Download Duke3D shareware for testing"
	@echo ""
	@echo "Variables:"
	@echo "  BUILD_TYPE=Debug|Release (default: Release)"
	@echo "  JOBS=n                   (default: all CPUs)"
