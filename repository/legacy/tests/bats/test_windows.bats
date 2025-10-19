#!/usr/bin/env bats
# Windows tests for art2img using BATS framework

# Load common utilities
load "./common.bats"

# Override wine execution functions to run from correct working directory
execute_wine_command_bats() {
    local cmd="$1"
    local description="${2:-Wine command}"
    local allow_failure="${3:-false}"

    # Extract executable path and arguments
    local executable_path
    local args
    executable_path=$(echo "$cmd" | cut -d' ' -f1)
    args=$(echo "$cmd" | cut -d' ' -f2-)

    # Get absolute path for build directory 
    local abs_build_dir="$(cd "${BUILD_DIR}" && pwd)"
    
    # Convert executable path to relative path for execution from build directory
    local relative_executable="$executable_path"
    if [[ "$executable_path" == "$abs_build_dir/"* ]]; then
        relative_executable="./${executable_path#"${abs_build_dir}/"}"
    fi

    # Reconstruct the wine command with relative executable path
    local relative_cmd="$relative_executable"
    if [[ -n "$args" ]]; then
        relative_cmd="$relative_cmd $args"
    fi

    # Run Wine command from build directory to ensure correct path resolution
    local full_cmd="cd '${BUILD_DIR}' && wine ${relative_cmd}"

    execute_platform_command "$full_cmd" "$description" "$allow_failure"
}

# Test Windows-specific functionality
@test "test_windows_basic_functionality" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform (but account for actual Windows vs Wine)
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    log_info "Testing Windows binary basic functionality..."
    
    # Test version
    run execute_wine_command_bats "$MAIN_BIN --version" "Windows version check"
    [ "$status" -eq 0 ] || {
        log_error "Windows version check failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Test help
    run execute_wine_command_bats "$MAIN_BIN --help" "Windows help output"
    [ "$status" -eq 0 ] || {
        log_error "Windows help output failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    log_success "Windows basic functionality tests completed"
}

@test "test_windows_unit_tests" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    log_info "Running Windows unit tests..."
    
    run execute_wine_command_bats "$TEST_BIN --no-colors" "Windows unit tests"
    [ "$status" -eq 0 ] || {
        log_error "Windows unit tests failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    log_success "Windows unit tests completed"
}

@test "test_windows_png_extraction" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/windows/png"
    create_directory_if_needed "$output_dir"
    
    log_info "Testing Windows PNG extraction..."
    
    # Use relative paths for files when executing from build directory
    local cmd_args="-o $output_dir -f png -p $PALETTE_FILE $TEST_ART_FILE"
    run execute_wine_command_bats "$MAIN_BIN $cmd_args" "Windows PNG extraction"
    [ "$status" -eq 0 ] || {
        log_error "Windows PNG extraction failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.png" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Windows PNG extraction successful! $file_count files created in $output_dir"
    else
        log_error "Test failed! No PNG files found in $output_dir"
        false
    fi
}

@test "test_windows_tga_extraction" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/windows/tga"
    create_directory_if_needed "$output_dir"
    
    log_info "Testing Windows TGA extraction..."
    
    # Use relative paths for files when executing from build directory
    local cmd_args="-o $output_dir -f tga -p $PALETTE_FILE $TEST_ART_FILE"
    run execute_wine_command_bats "$MAIN_BIN $cmd_args" "Windows TGA extraction"
    [ "$status" -eq 0 ] || {
        log_error "Windows TGA extraction failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.tga" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Windows TGA extraction successful! $file_count files created in $output_dir"
    else
        log_error "Test failed! No TGA files found in $output_dir"
        false
    fi
}

@test "test_windows_transparent_png_extraction" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/windows/transparent_png"
    create_directory_if_needed "$output_dir"
    
    log_info "Testing Windows transparent PNG extraction..."
    
    # Use relative paths for files when executing from build directory
    local cmd_args="-o $output_dir -f png -p $PALETTE_FILE $TEST_ART_FILE"
    run execute_wine_command_bats "$MAIN_BIN $cmd_args" "Windows transparent PNG extraction"
    [ "$status" -eq 0 ] || {
        log_error "Windows transparent PNG extraction failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.png" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Windows transparent PNG extraction successful! $file_count files created in $output_dir"
    else
        log_error "Test failed! No PNG files found in $output_dir"
        false
    fi
}

@test "test_windows_non_transparent_png_extraction" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    # Create output directory
    local output_dir="$OUTPUT_DIR/windows/non_transparent_png"
    create_directory_if_needed "$output_dir"
    
    log_info "Testing Windows non-transparent PNG extraction..."
    
    # Use relative paths for files when executing from build directory
    local cmd_args="-o $output_dir -f png -p $PALETTE_FILE --no-fix-transparency $TEST_ART_FILE"
    run execute_wine_command_bats "$MAIN_BIN $cmd_args" "Windows non-transparent PNG extraction"
    [ "$status" -eq 0 ] || {
        log_error "Windows non-transparent PNG extraction failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Check results
    local file_count
    file_count=$(count_files_by_pattern "*.png" "$output_dir")
    
    if [[ $file_count -gt 0 ]]; then
        log_success "Windows non-transparent PNG extraction successful! $file_count files created in $output_dir"
    else
        log_error "Test failed! No PNG files found in $output_dir"
        false
    fi
}

@test "test_windows_output_consistency" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    # Create output directories
    local png_output_dir="$OUTPUT_DIR/windows/consistency_png"
    local tga_output_dir="$OUTPUT_DIR/windows/consistency_tga"
    local transp_output_dir="$OUTPUT_DIR/windows/consistency_transparent"
    local no_transp_output_dir="$OUTPUT_DIR/windows/consistency_non_transparent"
    
    create_directory_if_needed "$png_output_dir"
    create_directory_if_needed "$tga_output_dir"
    create_directory_if_needed "$transp_output_dir"
    create_directory_if_needed "$no_transp_output_dir"
    
    log_info "Testing Windows output consistency across formats..."
    
    # Run extractions
    run execute_wine_command_bats "$MAIN_BIN -o $png_output_dir -f png -p $PALETTE_FILE $TEST_ART_FILE" "PNG consistency test"
    [ "$status" -eq 0 ] || {
        log_error "PNG consistency test failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    run execute_wine_command_bats "$MAIN_BIN -o $tga_output_dir -f tga -p $PALETTE_FILE $TEST_ART_FILE" "TGA consistency test"
    [ "$status" -eq 0 ] || {
        log_error "TGA consistency test failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    run execute_wine_command_bats "$MAIN_BIN -o $transp_output_dir -f png -p $PALETTE_FILE $TEST_ART_FILE" "Transparent PNG consistency test"
    [ "$status" -eq 0 ] || {
        log_error "Transparent PNG consistency test failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    run execute_wine_command_bats "$MAIN_BIN -o $no_transp_output_dir -f png -p $PALETTE_FILE --no-fix-transparency $TEST_ART_FILE" "Non-transparent PNG consistency test"
    [ "$status" -eq 0 ] || {
        log_error "Non-transparent PNG consistency test failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Validate consistency
    local png_count=$(count_files_by_pattern "*.png" "$png_output_dir")
    local tga_count=$(count_files_by_pattern "*.tga" "$tga_output_dir")
    local transp_count=$(count_files_by_pattern "*.png" "$transp_output_dir")
    local no_transp_count=$(count_files_by_pattern "*.png" "$no_transp_output_dir")
    
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
        log_success "Windows output consistency validated"
    else
        log_error "Inconsistent output file counts"
        log_error "Expected: PNG=$tga_count, TRANSP=$transp_count, NO_TRANSP=$no_transp_count"
        log_error "Got: PNG=$png_count, TGA=$tga_count, TRANSP=$transp_count, NO_TRANSP=$no_transp_count"
        false
    fi
}

@test "test_windows_file_integrity" {
    # Validate test assets exist
    validate_test_assets || skip "Test assets not found"
    validate_binaries "$MAIN_BIN" "$TEST_BIN" || skip "Binaries not found"
    
    # Skip if not on Windows platform
    if [[ "$PLATFORM" != "windows" ]]; then
        skip "Skipping Windows-specific tests on non-Windows platform"
    fi
    
    # Create output directories
    local png_output_dir="$OUTPUT_DIR/windows/integrity_png"
    local tga_output_dir="$OUTPUT_DIR/windows/integrity_tga"
    
    create_directory_if_needed "$png_output_dir"
    create_directory_if_needed "$tga_output_dir"
    
    log_info "Testing Windows file integrity..."
    
    # Run extractions
    run execute_wine_command_bats "$MAIN_BIN -o $png_output_dir -f png -p $PALETTE_FILE $TEST_ART_FILE" "PNG integrity test"
    [ "$status" -eq 0 ] || {
        log_error "PNG integrity test failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    run execute_wine_command_bats "$MAIN_BIN -o $tga_output_dir -f tga -p $PALETTE_FILE $TEST_ART_FILE" "TGA integrity test"
    [ "$status" -eq 0 ] || {
        log_error "TGA integrity test failed with status: $status"
        log_error "Output: $output"
        false
    }
    
    # Validate file integrity
    local png_count=$(count_files_by_pattern "*.png" "$png_output_dir")
    local tga_count=$(count_files_by_pattern "*.tga" "$tga_output_dir")
    
    # Validate PNG files if any were created
    if [[ $png_count -gt 0 ]]; then
        local valid_png=$(count_valid_files_by_format "$png_output_dir" "*.png" "PNG")
        log_info "Valid PNG files: $valid_png/$png_count"
        
        if [[ $valid_png -ne $png_count ]]; then
            log_error "Some PNG files are corrupted"
            false
        fi
    fi
    
    # Validate TGA files if any were created
    if [[ $tga_count -gt 0 ]]; then
        local valid_tga=$(count_valid_files_by_format "$tga_output_dir" "*.tga" "TGA")
        log_info "Valid TGA files: $valid_tga/$tga_count"
        
        if [[ $valid_tga -ne $tga_count ]]; then
            log_error "Some TGA files are corrupted"
            false
        fi
    fi
    
    log_success "Windows file integrity validated"
}