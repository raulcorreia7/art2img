#!/bin/bash
set -e

# =============================================================================
# Windows Test Suite for art2img
# =============================================================================
# This script provides comprehensive testing for Windows builds using Wine
# Usage: ./scripts/test_windows.sh [functional-only]
# =============================================================================

# Source common utilities
readonly SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/test_common.sh"

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
readonly PROJECT_ROOT="$(detect_project_root)"

# Parse arguments: optional "functional-only" flag followed by build directories
FUNCTIONAL_ONLY="false"
ARGS=("$@")

if [[ "${ARGS[0]:-}" == "functional-only" ]]; then
    FUNCTIONAL_ONLY="functional-only"
    ARGS=("${ARGS[@]:1}")
fi

# Use provided build directory or detect it
if [[ -n "${ARGS[0]:-}" ]]; then
    BUILD_DIR="${ARGS[0]}"
else
    BUILD_DIR="$(detect_build_dir)"
fi

# Use provided Windows build directory or default to BUILD_DIR
if [[ -n "${ARGS[1]:-}" ]]; then
    WINDOWS_BUILD_DIR="${ARGS[1]}"
else
    WINDOWS_BUILD_DIR="${BUILD_DIR}"
fi

readonly FUNCTIONAL_ONLY BUILD_DIR WINDOWS_BUILD_DIR

readonly WINDOWS_BIN_DIR="${WINDOWS_BUILD_DIR}/bin"
readonly MAIN_BIN="${WINDOWS_BIN_DIR}/art2img.exe"
readonly TEST_BIN="${WINDOWS_BIN_DIR}/art2img_tests.exe"

# Test assets
readonly TEST_ASSETS_DIR="tests/assets"
readonly PALETTE_FILE="${TEST_ASSETS_DIR}/PALETTE.DAT"
readonly TEST_ART_FILE="${TEST_ASSETS_DIR}/TILES000.ART"

# Output directories
readonly OUTPUT_DIR="${BUILD_DIR}/tests/output"
readonly PNG_OUTPUT_DIR="${OUTPUT_DIR}/ci_png"
readonly TGA_OUTPUT_DIR="${OUTPUT_DIR}/ci_tga"
readonly TRANSP_OUTPUT_DIR="${OUTPUT_DIR}/ci_transparency"
readonly NO_TRANSP_OUTPUT_DIR="${OUTPUT_DIR}/ci_no_transparency"

# Test configuration
# -----------------------------------------------------------------------------
# Validation functions
# -----------------------------------------------------------------------------
validate_environment() {
    log_info "Validating test environment"
    
    validate_wine_installation || return 1
    
    # Validate binaries
    local missing_bins=()
    [[ ! -f "$MAIN_BIN" ]] && missing_bins+=("$MAIN_BIN")
    [[ ! -f "$TEST_BIN" ]] && missing_bins+=("$TEST_BIN")
    
    if [[ ${#missing_bins[@]} -gt 0 ]]; then
        log_error "Windows binaries not found:"
        printf "  %s\n" "${missing_bins[@]}"
        log_error "Build Windows binaries first:"
        log_error "  make windows-release"
        log_error "  make windows-release CMAKE_ARGS='-DBUILD_TESTS=ON'"
        return 1
    fi
    
    # Validate test assets
    validate_file_exists "$PALETTE_FILE" "Palette file" || return 1
    validate_file_exists "$TEST_ART_FILE" "Test ART file" || return 1
    
    log_success "Environment validation completed"
    return 0
}

# -----------------------------------------------------------------------------
# Test execution functions
# -----------------------------------------------------------------------------
test_basic_functionality() {
    log_step "Testing basic functionality"
    
    execute_wine_command_verbose "$MAIN_BIN --version" "Version check" || return 1
    execute_wine_command_verbose "$MAIN_BIN --help" "Help output" || return 1
    
    log_success "Basic functionality tests completed"
    return 0
}

test_unit_tests() {
    if [[ "$FUNCTIONAL_ONLY" == "functional-only" ]]; then
        log_info "Skipping unit tests (functional-only mode)"
        return 0
    fi
    
    log_step "Running unit tests"
    execute_wine_command_verbose "$TEST_BIN --no-colors" "Unit tests" || return 1
    
    log_success "Unit tests completed"
    return 0
}

test_extraction_format() {
    local format="$1"
    local output_dir="$2"
    local extra_args="$3"
    local description="$4"
    
    log_step "Testing $description"
    create_directory_if_needed "$output_dir"
    
    local cmd="$MAIN_BIN -o $output_dir -f $format -p $PALETTE_FILE $extra_args $TEST_ART_FILE"
    execute_wine_command "$cmd" "$description" || return 1
    
    log_success "$description completed"
    return 0
}

test_functional_suite() {
    log_step "Running functional test suite"
    
    # Test different formats and options
    test_extraction_format "png" "$PNG_OUTPUT_DIR" "" "PNG extraction" || return 1
    test_extraction_format "tga" "$TGA_OUTPUT_DIR" "" "TGA extraction" || return 1
    test_extraction_format "png" "$TRANSP_OUTPUT_DIR" "" "Transparent PNG extraction" || return 1
    test_extraction_format "png" "$NO_TRANSP_OUTPUT_DIR" "-N" "Non-transparent PNG extraction" || return 1
    
    log_success "Functional test suite completed"
    return 0
}

# -----------------------------------------------------------------------------
# Validation functions
# -----------------------------------------------------------------------------
validate_output_consistency() {
    log_step "Validating output consistency"
    
    local png_count=$(count_files_by_pattern "*.png" "$PNG_OUTPUT_DIR")
    local tga_count=$(count_files_by_pattern "*.tga" "$TGA_OUTPUT_DIR")
    local transp_count=$(count_files_by_pattern "*.png" "$TRANSP_OUTPUT_DIR")
    local no_transp_count=$(count_files_by_pattern "*.png" "$NO_TRANSP_OUTPUT_DIR")
    
    log_info "Output file counts:"
    log_info "  PNG files: $png_count"
    log_info "  TGA files: $tga_count"
    log_info "  Transparent PNG files: $transp_count"
    log_info "  Non-transparent PNG files: $no_transp_count"
    
    # Validate consistency across all formats
    if [[ $png_count -eq $tga_count ]] && 
       [[ $png_count -eq $transp_count ]] && 
       [[ $png_count -eq $no_transp_count ]] && 
       [[ $png_count -gt 0 ]]; then
        log_success "Output consistency validated"
        echo "$png_count"
        return 0
    else
        log_error "Inconsistent output file counts"
        log_error "Expected: PNG=$tga_count, TRANSP=$transp_count, NO_TRANSP=$no_transp_count"
        log_error "Got: PNG=$png_count, TGA=$tga_count, TRANSP=$transp_count, NO_TRANSP=$no_transp_count"
        return 1
    fi
}

validate_file_integrity() {
    log_step "Validating file integrity"
    
    local png_count=$(count_files_by_pattern "*.png" "$PNG_OUTPUT_DIR")
    local tga_count=$(count_files_by_pattern "*.tga" "$TGA_OUTPUT_DIR")
    
    # Validate PNG files
    if [[ $png_count -gt 0 ]]; then
        local valid_png=$(count_valid_files_by_format "$PNG_OUTPUT_DIR" "*.png" "PNG")
        log_info "Valid PNG files: $valid_png/$png_count"
        
        if [[ $valid_png -ne $png_count ]]; then
            log_error "Some PNG files are corrupted"
            return 1
        fi
    fi
    
    # Validate TGA files
    if [[ $tga_count -gt 0 ]]; then
        local valid_tga=$(count_valid_files_by_format "$TGA_OUTPUT_DIR" "*.tga" "TGA")
        log_info "Valid TGA files: $valid_tga/$tga_count"
        
        if [[ $valid_tga -ne $tga_count ]]; then
            log_error "Some TGA files are corrupted"
            return 1
        fi
    fi
    
    log_success "File integrity validated"
    return 0
}

# -----------------------------------------------------------------------------
# Test execution orchestration
# -----------------------------------------------------------------------------
execute_test_suite() {
    local total_tests=0
    local passed_tests=0
    
    # Basic functionality tests
    ((total_tests++))
    if test_basic_functionality; then
        ((passed_tests++))
    fi
    
    # Unit tests
    ((total_tests++))
    if test_unit_tests; then
        ((passed_tests++))
    fi
    
    # Functional tests
    ((total_tests++))
    if test_functional_suite; then
        ((passed_tests++))
    fi
    
    # Output validation
    ((total_tests++))
    local tile_count
    if tile_count=$(validate_output_consistency); then
        ((passed_tests++))
    fi
    
    # File integrity validation
    ((total_tests++))
    if validate_file_integrity; then
        ((passed_tests++))
    fi
    
    create_test_summary "$total_tests" "$passed_tests" "$((total_tests - passed_tests))" "Windows Test Suite"
    local summary_status=$?

    echo "$tile_count"
    return $summary_status
}

# -----------------------------------------------------------------------------
# Main execution
# -----------------------------------------------------------------------------
main() {
    log_info "Starting Windows Test Suite for art2img"
    log_info "Project root: $PROJECT_ROOT"
    log_info "Build directory: $BUILD_DIR"
    log_info "Functional only mode: $FUNCTIONAL_ONLY"
    
    # Validate environment
    validate_environment || exit 1
    
    # Execute test suite
    local tile_count
    tile_count=$(execute_test_suite) || exit 1
    
    # Final success
    log_success "All Windows tests passed successfully"
    log_success "Processed $tile_count tiles"
    log_success "Wine testing completed"
    
    exit 0
}

# Execute main function
main "$@"
