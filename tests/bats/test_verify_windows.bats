#!/usr/bin/env bats
# Windows test setup verification for art2img using BATS framework

# Load common utilities
load "./common.bats"

# Setup: Set up variables for test
setup() {
    # Project root detection
    PROJECT_ROOT="$(cd "$(dirname "${BATS_TEST_FILENAME}")/../.." && pwd)"
    
    # Configuration
    : "${BUILD_DIR:=build}"
    : "${WINDOWS_TOOLCHAIN:=cmake/windows-toolchain.cmake}"
    : "${TEST_ASSETS_DIR:=tests/assets}"
    WINDOWS_BIN_DIR="${BUILD_DIR}/windows-release/bin"
    
    # Binary paths
    MAIN_BIN="${WINDOWS_BIN_DIR}/art2img.exe"
    TEST_BIN="${WINDOWS_BIN_DIR}/art2img_tests.exe"
    
    # Test assets
    PALETTE_FILE="${TEST_ASSETS_DIR}/PALETTE.DAT"
    TEST_ART_FILE="${TEST_ASSETS_DIR}/TILES000.ART"
}

# Test Windows setup verification
@test "test_windows_setup_system_requirements" {
    log_info "Testing validation of system requirements for Windows testing"
    
    # Validate Wine is available
    run validate_command_exists "wine" "Wine"
    [ "$status" -eq 0 ] || {
        log_error "Wine is not installed. Install with:"
        log_error "  Ubuntu/Debian: sudo apt-get install wine wine64"
        log_error "  Fedora: sudo dnf install wine"
        log_error "  Arch: sudo pacman -S wine"
        false
    }
    
    # Validate MinGW cross-compiler is available (if needed)
    run validate_command_exists "x86_64-w64-mingw32-g++" "MinGW cross-compiler"
    [ "$status" -eq 0 ] || {
        log_warn "MinGW cross-compiler not found. For cross-compilation, install with:"
        log_warn "  Ubuntu/Debian: sudo apt-get install g++-mingw-w64-x86-64"
        log_warn "  Fedora: sudo dnf install mingw64-gcc"
        log_warn "  Arch: sudo pacman -S mingw-w64-gcc"
    }
    
    log_success "System requirements validation completed"
}

@test "test_windows_setup_project_structure" {
    log_info "Testing validation of project structure for Windows testing"
    
    # Validate toolchain file
    run validate_file_exists "$WINDOWS_TOOLCHAIN" "Windows toolchain file"
    [ "$status" -eq 0 ] || {
        log_error "Windows toolchain file not found: $WINDOWS_TOOLCHAIN"
        false
    }
    
    # Validate common utilities (BATS version)
    run validate_file_exists "tests/bats/common.bats" "Common BATS test utilities"
    [ "$status" -eq 0 ] || {
        log_error "Common BATS test utilities not found: tests/bats/common.bats"
        false
    }
    
    # Validate Makefile targets exist
    run grep -q "^test-windows:" Makefile
    [ "$status" -eq 0 ] || {
        log_error "Makefile target not found: test-windows"
    }
    
    # Validate test assets
    run validate_file_exists "$PALETTE_FILE" "Palette file"
    [ "$status" -eq 0 ] || {
        log_error "Palette file not found: $PALETTE_FILE"
        false
    }
    
    run validate_file_exists "$TEST_ART_FILE" "Test ART file"
    [ "$status" -eq 0 ] || {
        log_error "Test ART file not found: $TEST_ART_FILE"
        false
    }
    
    log_success "Project structure validation completed"
}

@test "test_windows_setup_binary_availability" {
    log_info "Testing validation of Windows binary availability"
    
    # This test checks if the binaries exist
    # On non-Windows platforms, these binaries may not be built, which is expected
    
    local missing_bins=0
    
    if [[ ! -f "$MAIN_BIN" ]]; then
        log_warn "Windows main binary not found: $MAIN_BIN"
        log_info "To build Windows binaries: make windows-release"
        missing_bins=$((missing_bins + 1))
    else
        log_success "Windows main binary found: $MAIN_BIN"
    fi
    
    if [[ ! -f "$TEST_BIN" ]]; then
        log_warn "Windows test binary not found: $TEST_BIN"
        log_info "To build Windows test binaries: make windows-release CMAKE_ARGS='-DBUILD_TESTS=ON'"
        missing_bins=$((missing_bins + 1))
    else
        log_success "Windows test binary found: $TEST_BIN"
    fi
    
    if [[ $missing_bins -eq 0 ]]; then
        log_success "Windows binaries are available"
    else
        log_info "Note: One or more Windows binaries are missing - this is expected on non-Windows platforms or if you haven't built them yet"
        # Don't fail the test since missing Windows binaries on Linux is expected
        log_success "Windows binary availability check completed (expected missing on non-Windows)"
    fi
}

@test "test_windows_setup_configuration_files" {
    log_info "Testing validation of Windows test configuration files"
    
    # Check for Makefile
    run validate_file_exists "Makefile" "Makefile"
    [ "$status" -eq 0 ] || {
        log_error "Makefile not found"
        false
    }
    
    # Check for CMake configuration
    run validate_file_exists "CMakeLists.txt" "Main CMake file"
    [ "$status" -eq 0 ] || {
        log_error "Main CMake file not found: CMakeLists.txt"
        false
    }
    
    # Check for Windows toolchain
    run validate_file_exists "$WINDOWS_TOOLCHAIN" "Windows toolchain"
    [ "$status" -eq 0 ] || {
        log_error "Windows toolchain not found: $WINDOWS_TOOLCHAIN"
        false
    }
    
    log_success "Configuration files validation completed"
}

@test "test_windows_setup_troubleshooting_info" {
    log_info "Testing availability of troubleshooting information for Windows setup"
    
    # Check if documentation exists
    run validate_file_exists "docs/building.md" "Building documentation" "false"
    if [ "$status" -eq 0 ]; then
        log_info "Building documentation available: docs/building.md"
    else
        log_warn "Building documentation not found: docs/building.md"
    fi
    
    # Show recommended commands if running on a compatible system
    if command -v wine >/dev/null 2>&1; then
        log_info "To build Windows binaries: make windows-release"
        log_info "To build with tests: make windows-release CMAKE_ARGS='-DBUILD_TESTS=ON'"
        log_info "To run Windows tests: make test-windows"
    fi
    
    log_success "Troubleshooting information check completed"
}