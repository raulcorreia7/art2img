#!/bin/bash
set -e

# =============================================================================
# Windows Test Setup Verification Script
# =============================================================================
# This script verifies that Windows testing is properly configured
# Usage: ./scripts/verify_windows_tests.sh
# =============================================================================

# Source common utilities
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/test_common.sh"

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
readonly PROJECT_ROOT="$(detect_project_root)"
readonly BUILD_DIR="$(detect_build_dir)"
readonly WINDOWS_TOOLCHAIN="${WINDOWS_TOOLCHAIN:-cmake/windows-toolchain.cmake}"
readonly TEST_ASSETS_DIR="${TEST_ASSETS_DIR:-tests/assets}"
readonly WINDOWS_BIN_DIR="${BUILD_DIR}/windows-release/bin"

# Binary paths
readonly MAIN_BIN="${WINDOWS_BIN_DIR}/art2img.exe"
readonly TEST_BIN="${WINDOWS_BIN_DIR}/art2img_tests.exe"

# Test assets
readonly PALETTE_FILE="${TEST_ASSETS_DIR}/PALETTE.DAT"
readonly TEST_ART_FILE="${TEST_ASSETS_DIR}/TILES000.ART"

# -----------------------------------------------------------------------------
# Validation functions
# -----------------------------------------------------------------------------
validate_system_requirements() {
    log_info "Validating system requirements"
    
    local errors=0
    
    validate_command_exists "wine" "Wine" || ((errors++))
    validate_command_exists "x86_64-w64-mingw32-g++" "MinGW cross-compiler" || ((errors++))
    
    if [[ $errors -gt 0 ]]; then
        log_error "System requirements validation failed"
        return 1
    fi
    
    log_success "System requirements validated"
    return 0
}

validate_project_structure() {
    log_info "Validating project structure"
    
    local errors=0
    
    # Validate toolchain file
    validate_file_exists "$WINDOWS_TOOLCHAIN" "Windows toolchain file" || ((errors++))
    
    # Validate test script
    validate_file_exists "scripts/test_windows.sh" "Windows test script" || ((errors++))
    
    # Make sure test script is executable
    if [[ -f "scripts/test_windows.sh" && ! -x "scripts/test_windows.sh" ]]; then
        log_warn "Making test script executable"
        chmod +x "scripts/test_windows.sh"
    fi
    
    # Validate common utilities
    validate_file_exists "scripts/test_common.sh" "Common test utilities" || ((errors++))
    
    # Validate Makefile targets
    if ! grep -q "^test-windows:" Makefile; then
        log_error "Makefile target not found: test-windows"
        ((errors++))
    fi
    
    if ! grep -q "^verify-setup:" Makefile; then
        log_error "Makefile target not found: verify-setup"
        ((errors++))
    fi
    
    # Validate test assets
    validate_file_exists "$PALETTE_FILE" "Palette file" || ((errors++))
    validate_file_exists "$TEST_ART_FILE" "Test ART file" || ((errors++))
    
    if [[ $errors -gt 0 ]]; then
        log_error "Project structure validation failed"
        return 1
    fi
    
    log_success "Project structure validated"
    return 0
}

validate_build_environment() {
    log_info "Validating build environment"
    
    # Validate build directory
    validate_directory_exists "$BUILD_DIR" "Build directory" "false"
    
    # Validate Wine functionality
    validate_wine_functionality || return 1
    
    log_success "Build environment validated"
    return 0
}

validate_binary_availability() {
    log_info "Validating Windows binaries"
    
    local missing_bins=0
    
    validate_file_exists "$MAIN_BIN" "Windows main binary" "false" || ((missing_bins++))
    validate_file_exists "$TEST_BIN" "Windows test binary" "false" || ((missing_bins++))
    
    if [[ $missing_bins -gt 0 ]]; then
        log_warn "Some binaries are missing"
        log_info "Build Windows binaries with:"
        log_info "  make windows-release"
        log_info "  make windows-release CMAKE_ARGS='-DBUILD_TESTS=ON'"
        return 0  # Not a failure, just a warning
    fi
    
    log_success "Windows binaries validated"
    return 0
}

validate_configuration_files() {
    log_info "Validating configuration files"
    
    local errors=0
    
    # Check for documentation
    validate_file_exists "docs/building.md" "Building documentation" "false" || ((errors++))
    
    # Check for Makefile
    validate_file_exists "Makefile" "Makefile" || ((errors++))
    
    # Check for CMake configuration
    validate_file_exists "CMakeLists.txt" "Main CMake file" || ((errors++))
    validate_file_exists "$WINDOWS_TOOLCHAIN" "Windows toolchain" || ((errors++))
    
    if [[ $errors -gt 0 ]]; then
        log_error "Configuration validation failed"
        return 1
    fi
    
    log_success "Configuration files validated"
    return 0
}

# -----------------------------------------------------------------------------
# Recommendations and help
# -----------------------------------------------------------------------------
provide_setup_recommendations() {
    log_info "Setup recommendations"
    
    if [[ ! -f "$MAIN_BIN" ]] || [[ ! -f "$TEST_BIN" ]]; then
        log_info "To build Windows binaries:"
        log_info "  make windows-release"
        log_info "  make windows-release CMAKE_ARGS='-DBUILD_TESTS=ON'"
    fi
    
    if [[ -f "$MAIN_BIN" && -f "$TEST_BIN" ]]; then
        log_info "To run Windows tests:"
        log_info "  make test-windows"
        log_info "  make test-windows-unit"
        log_info "  make test-windows-functional"
    fi
    
    log_info "For troubleshooting, see: docs/building.md"
}

provide_troubleshooting_info() {
    log_info "Troubleshooting information"
    
    # Check common issues
    if ! validate_command_exists "wine" "Wine" "false"; then
        log_info "Wine installation:"
        log_info "  Ubuntu/Debian: sudo apt-get install wine wine64"
        log_info "  Fedora: sudo dnf install wine"
        log_info "  Arch: sudo pacman -S wine"
    fi
    
    if ! validate_command_exists "x86_64-w64-mingw32-g++" "MinGW" "false"; then
        log_info "MinGW installation:"
        log_info "  Ubuntu/Debian: sudo apt-get install g++-mingw-w64-x86-64"
        log_info "  Fedora: sudo dnf install mingw64-gcc"
        log_info "  Arch: sudo pacman -S mingw-w64-gcc"
    fi
}

# -----------------------------------------------------------------------------
# Validation orchestration
# -----------------------------------------------------------------------------
execute_validation_suite() {
    local total_checks=0
    local passed_checks=0
    
    # System requirements
    ((total_checks++))
    if validate_system_requirements; then
        ((passed_checks++))
    fi
    
    # Project structure
    ((total_checks++))
    if validate_project_structure; then
        ((passed_checks++))
    fi
    
    # Configuration files
    ((total_checks++))
    if validate_configuration_files; then
        ((passed_checks++))
    fi
    
    # Build environment
    ((total_checks++))
    if validate_build_environment; then
        ((passed_checks++))
    fi
    
    # Binary availability
    ((total_checks++))
    if validate_binary_availability; then
        ((passed_checks++))
    fi
    
    create_test_summary "$total_checks" "$passed_checks" "$((total_checks - passed_checks))" "Setup Verification"
}

# -----------------------------------------------------------------------------
# Main execution
# -----------------------------------------------------------------------------
main() {
    log_info "Starting Windows Test Setup Verification"
    log_info "Project root: $PROJECT_ROOT"
    log_info "Build directory: $BUILD_DIR"
    
    # Execute validation suite
    execute_validation_suite || exit 1
    
    # Provide recommendations
    provide_setup_recommendations
    provide_troubleshooting_info
    
    # Final success
    log_success "Setup verification completed"
    
    exit 0
}

# Execute main function
main "$@"