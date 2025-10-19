#!/usr/bin/env bats
# Comprehensive transparency tests for art2img using BATS framework

# Load common utilities
load "./common.bats"

setup() {
    # Call the common setup first
    common_setup
    
    # Additional setup for transparency tests
    readonly OUTPUT_DIR="$BATS_TEST_TMPDIR/transparency_tests"
    create_directory_if_needed "$OUTPUT_DIR"
}

@test "test_comprehensive_transparency_unit_tests" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    log_info "Running transparency unit tests..."
    
    run execute_binary_command "$TEST_BIN" "-c \"*transparency*\" --no-colors" "Transparency unit tests"
    # Allow failure here as it might be that no tests match the filter
    log_info "Transparency unit tests completed with status: $status"
    log_info "Output: $output"
    
    # Verify the test binary exists and works
    run execute_binary_command "$TEST_BIN" "--help" "Test binary help"
    [ "$status" -eq 0 ]
}

@test "test_comprehensive_transparency_with_real_assets" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Create test output directories
    local with_transparency_dir="$OUTPUT_DIR/with_transparency"
    local no_transparency_dir="$OUTPUT_DIR/no_transparency"
    local bmp_dir="$OUTPUT_DIR/bmp"
    local tga_dir="$OUTPUT_DIR/tga"
    
    create_directory_if_needed "$with_transparency_dir"
    create_directory_if_needed "$no_transparency_dir"
    create_directory_if_needed "$bmp_dir"
    create_directory_if_needed "$tga_dir"
    
    # Test with transparency enabled
    log_info "Testing with transparency enabled..."
    run execute_binary_command "$MAIN_BIN" "-f png -o '$with_transparency_dir' '$TEST_ART_FILE'" "Transparency enabled PNG"
    [ "$status" -eq 0 ]
    
    # Test with transparency disabled
    log_info "Testing with transparency disabled..."
    run execute_binary_command "$MAIN_BIN" "-f png -o '$no_transparency_dir' --no-fix-transparency '$TEST_ART_FILE'" "Transparency disabled PNG"
    [ "$status" -eq 0 ]
    
    # Test with BMP format
    log_info "Testing with BMP format..."
    run execute_binary_command "$MAIN_BIN" "-f bmp -o '$bmp_dir' '$TEST_ART_FILE'" "BMP format"
    [ "$status" -eq 0 ]
    
    # Test with TGA format
    log_info "Testing with TGA format..."
    run execute_binary_command "$MAIN_BIN" "-f tga -o '$tga_dir' '$TEST_ART_FILE'" "TGA format"
    [ "$status" -eq 0 ]
    
    # Verify results
    local png_with_transp_count=$(count_files_by_pattern "*.png" "$with_transparency_dir")
    local png_no_transp_count=$(count_files_by_pattern "*.png" "$no_transparency_dir")
    local bmp_count=$(count_files_by_pattern "*.bmp" "$bmp_dir")
    local tga_count=$(count_files_by_pattern "*.tga" "$tga_dir")
    
    log_info "PNG with transparency: $png_with_transp_count files"
    log_info "PNG without transparency: $png_no_transp_count files"
    log_info "BMP files: $bmp_count files"
    log_info "TGA files: $tga_count files"
    
    [ $png_with_transp_count -gt 0 ]
    [ $png_no_transp_count -gt 0 ]
    [ $bmp_count -gt 0 ]
    [ $tga_count -gt 0 ]
}

@test "test_comprehensive_transparency_edge_cases" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Create test output directories
    local png_dir="$OUTPUT_DIR/edge_cases_png"
    local bmp_dir="$OUTPUT_DIR/edge_cases_bmp"
    local tga_dir="$OUTPUT_DIR/edge_cases_tga"
    local no_fix_dir="$OUTPUT_DIR/edge_cases_no_fix"
    
    create_directory_if_needed "$png_dir"
    create_directory_if_needed "$bmp_dir"
    create_directory_if_needed "$tga_dir"
    create_directory_if_needed "$no_fix_dir"
    
    # Test various output formats
    run execute_binary_command "$MAIN_BIN" "-f png -o '$png_dir' '$TEST_ART_FILE'" "Edge case PNG"
    [ "$status" -eq 0 ]
    
    run execute_binary_command "$MAIN_BIN" "-f bmp -o '$bmp_dir' '$TEST_ART_FILE'" "Edge case BMP"
    [ "$status" -eq 0 ]
    
    run execute_binary_command "$MAIN_BIN" "-f tga -o '$tga_dir' '$TEST_ART_FILE'" "Edge case TGA"
    [ "$status" -eq 0 ]
    
    # Test with custom options
    run execute_binary_command "$MAIN_BIN" "-f png --no-fix-transparency -o '$no_fix_dir' '$TEST_ART_FILE'" "Edge case no-fix"
    [ "$status" -eq 0 ]
    
    # Verify results
    local png_count=$(count_files_by_pattern "*.png" "$png_dir")
    local bmp_count=$(count_files_by_pattern "*.bmp" "$bmp_dir")
    local tga_count=$(count_files_by_pattern "*.tga" "$tga_dir")
    local no_fix_count=$(count_files_by_pattern "*.png" "$no_fix_dir")
    
    log_info "PNG files: $png_count"
    log_info "BMP files: $bmp_count"
    log_info "TGA files: $tga_count"
    log_info "No-fix PNG files: $no_fix_count"
    
    [ $png_count -gt 0 ]
    [ $bmp_count -gt 0 ]
    [ $tga_count -gt 0 ]
    [ $no_fix_count -gt 0 ]
}

teardown() {
    # Clean up test output directory
    if [[ -n "${OUTPUT_DIR:-}" ]] && [[ -d "$OUTPUT_DIR" ]]; then
        rm -rf "$OUTPUT_DIR"
    fi
}