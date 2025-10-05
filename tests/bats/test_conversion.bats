#!/usr/bin/env bats
# Conversion tests for art2img using BATS framework

# Load common utilities
load "./common.bats"

# Test conversion functionality
@test "test_conversion_png" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/test_conversion"
    create_directory_if_needed "$output_dir"
    
    # Run conversion
    log_info "Running art2img conversion test..."
    
    run execute_binary_command "$MAIN_BIN" "-f png -o '$output_dir' '$TEST_ART_FILE'" "PNG conversion"
    
    # Check command executed successfully
    [ "$status" -eq 0 ] || {
        log_error "Conversion command failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.png" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Test successful! $file_count PNG files created in $output_dir"
    else
        log_error "Test failed! No PNG files found in $output_dir"
        false
    fi
}

@test "test_conversion_tga" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/test_conversion_tga"
    create_directory_if_needed "$output_dir"
    
    # Run conversion
    log_info "Running art2img TGA conversion test..."
    
    run execute_binary_command "$MAIN_BIN" "-f tga -o '$output_dir' '$TEST_ART_FILE'" "TGA conversion"
    
    # Check command executed successfully
    [ "$status" -eq 0 ] || {
        log_error "TGA conversion command failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.tga" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Test successful! $file_count TGA files created in $output_dir"
    else
        log_error "Test failed! No TGA files found in $output_dir"
        false
    fi
}

@test "test_conversion_bmp" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/test_conversion_bmp"
    create_directory_if_needed "$output_dir"
    
    # Run conversion
    log_info "Running art2img BMP conversion test..."
    
    run execute_binary_command "$MAIN_BIN" "-f bmp -o '$output_dir' '$TEST_ART_FILE'" "BMP conversion"
    
    # Check command executed successfully
    [ "$status" -eq 0 ] || {
        log_error "BMP conversion command failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.bmp" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Test successful! $file_count BMP files created in $output_dir"
    else
        log_error "Test failed! No BMP files found in $output_dir"
        false
    fi
}