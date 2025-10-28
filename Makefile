# Streamlined Makefile wrapper for CMake
# Provides essential build targets for art2img project
# Follows GNU Make best practices

# ============================================================================
# Configuration Variables
# ============================================================================
BUILD_DIR ?= build
BUILD_TYPE ?= Release
JOBS ?= $(shell nproc)
CMAKE := cmake
FIND := find
XARGS := xargs

# ============================================================================
# PHONY Targets
# ============================================================================
.PHONY: all build clean test install help fmt fmt-check lint
.PHONY: windows-x64-mingw windows-x86-mingw windows
.PHONY: macos-x64-osxcross macos-arm64-osxcross macos
.PHONY: check-mingw check-osxcross

# ============================================================================
# Default Target
# ============================================================================
all: build

# ============================================================================
# Build Targets
# ============================================================================

# Native Linux build
build:
	@$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(JOBS)

# ============================================================================
# Cross-compilation Support Functions
# ============================================================================

# Check for required cross-compilation tools
define check_tool
	@which $(1) > /dev/null 2>&1 || \
		(echo "$(2) not found. $(3)"; exit 1)
endef

# ============================================================================
# Prerequisite Checks
# ============================================================================
check-mingw:
	$(call check_tool,x86_64-w64-mingw32-gcc,MinGW,Install with: sudo apt-get install mingw-w64)

check-osxcross:
	$(call check_tool,o64-clang,osxcross,Install osxcross first)

# ============================================================================
# Cross-compilation Targets
# ============================================================================

# Windows cross-compilation targets
windows-x64-mingw: check-mingw
	@$(CMAKE) -B $(BUILD_DIR)/windows-x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/windows_x64.cmake
	@$(CMAKE) --build $(BUILD_DIR)/windows-x64 --parallel $(JOBS)

windows-x86-mingw: check-mingw
	@$(CMAKE) -B $(BUILD_DIR)/windows-x86 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/windows_x86.cmake
	@$(CMAKE) --build $(BUILD_DIR)/windows-x86 --parallel $(JOBS)

# macOS cross-compilation targets
macos-x64-osxcross: check-osxcross
	@$(CMAKE) -B $(BUILD_DIR)/macos-x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/macos_x64.cmake
	@$(CMAKE) --build $(BUILD_DIR)/macos-x64 --parallel $(JOBS)

macos-arm64-osxcross: check-osxcross
	@$(CMAKE) -B $(BUILD_DIR)/macos-arm64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/macos_arm64.cmake
	@$(CMAKE) --build $(BUILD_DIR)/macos-arm64 --parallel $(JOBS)

# ============================================================================
# Platform Aliases
# ============================================================================
windows: windows-x64-mingw windows-x86-mingw
macos: macos-x64-osxcross macos-arm64-osxcross

# ============================================================================
# Development Targets
# ============================================================================

# Run tests (Linux native)
test: build
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel $(JOBS)

# Run integration tests only (tests 19-21)
test-intg: build
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel $(JOBS) -I 19,21

# Run unit tests only (tests 1-18)
test-unit: build
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel $(JOBS) -I 1,18

# Run smoke tests only (tests 22-29)
test-smoke: build
	@cd $(BUILD_DIR) && ctest --output-on-failure --parallel $(JOBS) -I 22,29



# Cross-compilation test targets
test-windows-x64: windows-x64-mingw
	@echo "Testing Windows x64 build..."
	@if command -v wine >/dev/null 2>&1; then \
		wine $(BUILD_DIR)/windows-x64/tests/art2img_tests.exe --help >/dev/null 2>&1 && \
		echo "Windows x64 test executable runs successfully"; \
	else \
		echo "Wine not available, cannot test Windows executable"; \
	fi

test-windows-x86: windows-x86-mingw
	@echo "Testing Windows x86 build..."
	@if command -v wine >/dev/null 2>&1; then \
		wine $(BUILD_DIR)/windows-x86/tests/art2img_tests.exe --help >/dev/null 2>&1 && \
		echo "Windows x86 test executable runs successfully"; \
	else \
		echo "Wine not available, cannot test Windows executable"; \
	fi

test-macos-x64: macos-x64-osxcross
	@echo "Testing macOS x64 build..."
	@if [ -f "$(BUILD_DIR)/macos-x64/tests/art2img_tests" ]; then \
		echo "macOS x64 test executable built successfully"; \
	else \
		echo "macOS x64 test executable not found"; \
	fi

test-macos-arm64: macos-arm64-osxcross
	@echo "Testing macOS ARM64 build..."
	@if [ -f "$(BUILD_DIR)/macos-arm64/tests/art2img_tests" ]; then \
		echo "macOS ARM64 test executable built successfully"; \
	else \
		echo "macOS ARM64 test executable not found"; \
	fi

# Platform test aliases
test-windows: test-windows-x64 test-windows-x86
test-macos: test-macos-x64 test-macos-arm64

# Install project
install: build
	@cd $(BUILD_DIR) && $(CMAKE) --install .

# Clean build directory
clean:
	@echo "Cleaning build directories..."
	@rm -rf $(BUILD_DIR)/
	@echo "Build directories cleaned successfully"

# ============================================================================
# Code Quality Targets
# ============================================================================

# Source file patterns
SRCDIRS := src include tests cli
CPPFILES := $(shell $(FIND) $(SRCDIRS) -name '*.cpp' -o -name '*.hpp' 2>/dev/null)

# Format code (src, include, and tests directories)
fmt:
	@echo "Formatting source code..."
	@$(FIND) $(SRCDIRS) -name '*.cpp' -o -name '*.hpp' | $(XARGS) -P $(JOBS) clang-format -i
	@echo "Code formatted successfully"

# Check formatting without modifying files
fmt-check:
	@echo "Checking code formatting..."
	@$(FIND) $(SRCDIRS) -name '*.cpp' -o -name '*.hpp' | $(XARGS) clang-format --dry-run --Werror
	@echo "Code formatting check passed"

# Run linting using clang-tidy
lint:
	@echo "Running static analysis..."
	@$(CMAKE) -B $(BUILD_DIR)/lint -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTING=OFF
	@cd $(BUILD_DIR)/lint && run-clang-tidy -config '{}' -p . $$(shell $(FIND) .. -name '*.cpp' -o -name '*.hpp') || true
	@echo "Static analysis completed"

# Generate code coverage report
coverage: build
	@echo "Generating code coverage report..."
	@cd $(BUILD_DIR) && \
		make art2img_tests && \
		./tests/art2img_tests && \
		gcovr --html-details coverage.html --root .. --print-summary

# ============================================================================
# Help Target
# ============================================================================
help:
	@echo "art2img - Modern CMake Build System"
	@echo ""
	@echo "USAGE:"
	@echo "  make [target] [BUILD_TYPE=Debug|Release] [JOBS=n]"
	@echo ""
	@echo "BUILD TARGETS:"
	@echo "  all                    - Build Linux x64 (default)"
	@echo "  build                  - Configure and build Linux x64"
	@echo "  install                - Install the project"
	@echo "  clean                  - Remove all build directories"
	@echo ""
	@echo "CROSS-COMPILATION:"
	@echo "  windows-x64-mingw      - Windows 64-bit cross-compile"
	@echo "  windows-x86-mingw      - Windows 32-bit cross-compile"
	@echo "  windows                - All Windows cross-compilation"
	@echo "  macos-x64-osxcross     - macOS Intel cross-compile"
	@echo "  macos-arm64-osxcross   - macOS ARM64 cross-compile"
	@echo "  macos                  - All macOS cross-compilation"
	@echo ""
	@echo "DEVELOPMENT:"
	@echo "  test                   - Run all tests (Linux) in parallel"
	@echo "  test-unit              - Run unit tests only in parallel"
	@echo "  test-intg              - Run integration tests only in parallel"
	@echo "  test-smoke             - Run smoke tests only in parallel"
	@echo "  test-windows           - Test Windows cross-compiled builds"
	@echo "  test-macos             - Test macOS cross-compiled builds"
	@echo "  coverage               - Generate code coverage report"
	@echo "  fmt                    - Format source code"
	@echo "  fmt-check              - Check formatting"
	@echo "  lint                   - Run static analysis"
	@echo "  help                   - Show this help message"
	@echo ""
	@echo "VARIABLES:"
	@echo "  BUILD_TYPE=$(BUILD_TYPE)    - Build type (Debug/Release)"
	@echo "  JOBS=$(JOBS)               - Number of parallel jobs"
	@echo "  BUILD_DIR=$(BUILD_DIR)      - Build directory"
	@echo ""
	@echo "EXAMPLES:"
	@echo "  make                      # Build with defaults"
	@echo "  make BUILD_TYPE=Debug     # Debug build"
	@echo "  make windows JOBS=8       # Cross-compile with 8 jobs"