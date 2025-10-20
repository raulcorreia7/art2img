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
	@$(CMAKE) -S . -B $(BUILD_DIR)/linux_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE)
	@$(CMAKE) --build $(BUILD_DIR)/linux_x64 --parallel $(JOBS)

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
	@$(CMAKE) -B $(BUILD_DIR)/windows_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/windows_x64.cmake
	@$(CMAKE) --build $(BUILD_DIR)/windows_x64 --parallel $(JOBS)

windows-x86-mingw: check-mingw
	@$(CMAKE) -B $(BUILD_DIR)/windows_x86 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/windows_x86.cmake
	@$(CMAKE) --build $(BUILD_DIR)/windows_x86 --parallel $(JOBS)

# macOS cross-compilation targets
macos-x64-osxcross: check-osxcross
	@$(CMAKE) -B $(BUILD_DIR)/macos_x64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/macos_x64.cmake
	@$(CMAKE) --build $(BUILD_DIR)/macos_x64 --parallel $(JOBS)

macos-arm64-osxcross: check-osxcross
	@$(CMAKE) -B $(BUILD_DIR)/macos_arm64 -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=cmake/macos_arm64.cmake
	@$(CMAKE) --build $(BUILD_DIR)/macos_arm64 --parallel $(JOBS)

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
	@cd $(BUILD_DIR)/linux_x64 && ctest --output-on-failure --parallel $(JOBS)

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
SRCDIRS := src include tests
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
	@echo "  test                   - Run tests (Linux)"
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