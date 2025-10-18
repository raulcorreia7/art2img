#!/usr/bin/env bats

# =============================================================================
# Common Test Utilities for art2img BATS Tests
# =============================================================================

# Load BATS support libraries (if available)
# This ensures proper support for testing functionality
find_binary_path() {
    local search_root="$1"
    local stem="$2"

    if [[ ! -d "$search_root" ]]; then
        echo ""
        return 1
    fi

    local match
    match=$(find "$search_root" -type f \( -name "$stem" -o -name "$stem.exe" \) -print -quit 2>/dev/null)

    if [[ -n "$match" ]]; then
        printf '%s' "$match"
        return 0
    fi

    echo ""
    return 1
}

# Common setup function
common_setup() {
    # Project root detection
    PROJECT_ROOT="$(cd "$(dirname "${BATS_TEST_FILENAME}")/../.." && pwd)"
    
    # Determine build type from environment or default to 'linux-x64'
    BUILD_TYPE="${BUILD_TYPE:-linux-x64}"
    
    # Set platform
    if [[ -n "${1:-}" ]]; then
        PLATFORM="$1"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$RUNNER_OS" == "Windows" ]] || [[ -n "${WINDIR:-}" ]]; then
        PLATFORM="windows"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        PLATFORM="macos"
    else
        PLATFORM="linux"
    fi
    
    # Set platform-specific build directory
    if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$RUNNER_OS" == "Windows" ]] || [[ -n "${WINDIR:-}" ]]; then
        if [[ "$BUILD_TYPE" == *"windows-x64"* ]]; then
            BUILD_SUBDIR="mingw-windows-x64"
        elif [[ "$BUILD_TYPE" == *"windows-x86"* ]]; then
            BUILD_SUBDIR="mingw-windows-x86"
        else
            BUILD_SUBDIR="windows"
        fi
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        BUILD_SUBDIR="macos-x64"
    else
        # For Linux, use linux-x64 as the default build directory
        if [[ "$BUILD_TYPE" == "linux" ]]; then
            BUILD_SUBDIR="linux-x64"
        else
            BUILD_SUBDIR="$BUILD_TYPE"
        fi
    fi
    
    # Set the main build directory based on the build subdirectory
    BUILD_DIR="${PROJECT_ROOT}/build/${BUILD_SUBDIR}"
    
    # Set platform-specific paths
    BINARY_DIR="${BUILD_DIR}/bin"
    
    # Test assets directory
    TEST_ASSETS_DIR="${PROJECT_ROOT}/tests/assets"
    PALETTE_FILE="${TEST_ASSETS_DIR}/PALETTE.DAT"
    TEST_ART_FILE="${TEST_ASSETS_DIR}/TILES000.ART"
    
    # Setup output directories in the proper build directory structure
    OUTPUT_DIR="${PROJECT_ROOT}/build/${BUILD_SUBDIR}/test_output"
    mkdir -p "$OUTPUT_DIR"
    
    # Set binary paths after all directories are set
    # Use direct paths instead of find_binary_path to avoid issues
    if [[ -f "$BINARY_DIR/art2img" ]]; then
        MAIN_BIN="$BINARY_DIR/art2img"
    elif [[ -f "$BINARY_DIR/art2img.exe" ]]; then
        MAIN_BIN="$BINARY_DIR/art2img.exe"
    else
        MAIN_BIN=""
    fi
    
    if [[ -f "$BINARY_DIR/art2img_tests" ]]; then
        TEST_BIN="$BINARY_DIR/art2img_tests"
    elif [[ -f "$BINARY_DIR/art2img_tests.exe" ]]; then
        TEST_BIN="$BINARY_DIR/art2img_tests.exe"
    else
        TEST_BIN=""
    fi
    
    # Debug output
    log_debug "PROJECT_ROOT: $PROJECT_ROOT"
    log_debug "BUILD_TYPE: $BUILD_TYPE"
    log_debug "BUILD_SUBDIR: $BUILD_SUBDIR"
    log_debug "BUILD_DIR: $BUILD_DIR"
    log_debug "BINARY_DIR: $BINARY_DIR"
    log_debug "MAIN_BIN: $MAIN_BIN"
    log_debug "TEST_BIN: $TEST_BIN"
}

# Default setup function for tests that don't override it
setup() {
    common_setup
}

teardown() {
    # Clean up test output directory
    if [[ -n "${OUTPUT_DIR:-}" ]] && [[ -d "$OUTPUT_DIR" ]]; then
        rm -rf "$OUTPUT_DIR"
    fi
}

# =============================================================================
# Logging utilities
# =============================================================================

# Set default empty values for colors (no tput to avoid CI issues)
RED=""
GREEN=""
YELLOW=""
BLUE=""
BOLD=""
RESET=""

# Make these variables available to functions that will be used
export RED GREEN YELLOW BLUE BOLD RESET

log_debug() { echo "${BLUE}[DEBUG]${RESET} $*" >&3; }
log_info() { echo "${BLUE}[INFO]${RESET} $*" >&3; }
log_warn() { echo "${YELLOW}[WARN]${RESET} $*" >&3; }
log_error() { echo "${RED}[ERROR]${RESET} $*" >&3; }
log_success() { echo "${GREEN}[PASS]${RESET} $*" >&3; }
log_test() { echo "${BOLD}[TEST]${RESET} $*" >&3; }
log_step() { echo "${BOLD}[STEP]${RESET} $*" >&3; }
log_check() { echo "${BLUE}[CHECK]${RESET} $*" >&3; }

# =============================================================================
# Execution utilities
# =============================================================================

# Execute command based on platform
execute_platform_command() {
    local cmd="$1"
    local description="${2:-Platform command}"
    local allow_failure="${3:-false}"
    local verbose="${4:-false}"
    
    log_test "Executing: $description"
    
    if [[ "$verbose" == "true" ]]; then
        log_debug "Command: $cmd"
    fi
    
    if eval "$cmd"; then
        log_success "Command succeeded: $description"
        return 0
    else
        if [[ "$allow_failure" == "true" ]]; then
            log_warn "Command failed (allowed): $description"
            return 0
        else
            log_error "Command failed: $description"
            return 1
        fi
    fi
}

# Execute command with Wine when needed for Windows binaries
execute_wine_command() {
    local cmd="$1"
    local description="${2:-Wine command}"
    local allow_failure="${3:-false}"
    local verbose="${4:-false}"
    
    local full_cmd="$cmd"
    
    # Use Wine for Windows binaries on non-Windows platforms 
    # when running on Linux/Mac with Wine installed
    if [[ "$PLATFORM" == "windows" ]]; then
        # On actual Windows, we don't need Wine
        # On Linux/Mac with Wine, we add the wine command prefix
        # Check if we're on a non-Windows host but testing Windows binaries
        if command -v wine >/dev/null 2>&1 && [[ "$RUNNER_OS" != "Windows" ]]; then
            full_cmd="wine $cmd"
        else
            # On actual Windows host, run command directly
            full_cmd="$cmd"
        fi
    fi
    
    if [[ "$verbose" == "true" ]]; then
        execute_platform_command "$full_cmd" "$description" "$allow_failure" true
    else
        execute_platform_command "$full_cmd" "$description" "$allow_failure" false
    fi
}

# Execute binary command with platform-specific handling
execute_binary_command() {
    local binary_path="$1"
    local args="$2"
    local description="${3:-Binary execution}"
    local allow_failure="${4:-false}"
    
    local full_cmd="${binary_path} ${args}"
    
    case "$PLATFORM" in
        "windows")
            # On actual Windows platforms vs cross-compilation with Wine
            if [[ "$RUNNER_OS" == "Windows" ]]; then
                # On actual Windows runner, run command directly
                execute_platform_command "$full_cmd" "$description" "$allow_failure"
            else
                # On Linux/Mac with Wine for Windows cross-compilation, use Wine
                execute_wine_command "$full_cmd" "$description" "$allow_failure"
            fi
            ;;
        *)
            execute_platform_command "$full_cmd" "$description" "$allow_failure"
            ;;
    esac
}

# =============================================================================
# Validation utilities
# =============================================================================

validate_command_exists() {
    local cmd="$1"
    local description="${2:-$cmd}"
    
    log_check "Validating $description"
    
    if command -v "$cmd" >/dev/null 2>&1; then
        log_success "$description found"
        return 0
    else
        log_error "$description not found"
        return 1
    fi
}

validate_file_exists() {
    local file_path="$1"
    local description="${2:-file}"
    local required="${3:-true}"
    
    log_check "Validating $description: $file_path"
    
    if [[ -f "$file_path" ]]; then
        log_success "$description found"
        return 0
    else
        if [[ "$required" == "true" ]]; then
            log_error "$description not found: $file_path"
            return 1
        else
            log_warn "$description not found: $file_path"
            return 0
        fi
    fi
}

validate_directory_exists() {
    local dir_path="$1"
    local description="${2:-directory}"
    local required="${3:-true}"
    
    log_check "Validating $description: $dir_path"
    
    if [[ -d "$dir_path" ]]; then
        log_success "$description found"
        return 0
    else
        if [[ "$required" == "true" ]]; then
            log_error "$description not found: $dir_path"
            return 1
        else
            log_warn "$description not found: $dir_path"
            return 0
        fi
    fi
}

# =============================================================================
# File utilities
# =============================================================================

count_files_by_pattern() {
    local pattern="$1"
    local directory="${2:-.}"
    
    find "$directory" -name "$pattern" 2>/dev/null | wc -l
}

validate_file_format() {
    local file="$1"
    local expected_format="$2"
    
    file "$file" 2>/dev/null | grep -q "$expected_format"
}

count_valid_files_by_format() {
    local directory="$1"
    local pattern="$2"
    local expected_format="$3"
    
    find "$directory" -name "$pattern" -exec sh -c '
        for file do
            if file "$file" 2>/dev/null | grep -q "'"$expected_format"'"; then
                echo "$file"
            fi
        done
    ' sh {} + | wc -l
}

create_directory_if_needed() {
    local dir_path="$1"
    
    if [[ ! -d "$dir_path" ]]; then
        log_debug "Creating directory: $dir_path"
        mkdir -p "$dir_path"
    fi
}

# =============================================================================
# Test asset utilities
# =============================================================================

validate_test_assets() {
    log_info "Validating test assets"
    
    validate_file_exists "$PALETTE_FILE" "Palette file" || return 1
    validate_file_exists "$TEST_ART_FILE" "Test ART file" || return 1
    
    log_success "Test assets validated"
    return 0
}

# =============================================================================
# Binary detection utilities
# =============================================================================

detect_binary_path() {
    local binary_name="$1"
    local build_dir="${2:-$BUILD_DIR}"
    
    case "$PLATFORM" in
        "windows")
            echo "$build_dir/bin/${binary_name}.exe"
            ;;
        *)
            echo "$build_dir/bin/$binary_name"
            ;;
    esac
}

validate_binaries() {
    local main_binary="$1"
    local test_binary="$2"
    
    log_info "Validating binaries for $PLATFORM platform"
    
    validate_file_exists "$main_binary" "Main binary" || return 1
    validate_file_exists "$test_binary" "Test binary" || return 1
    
    log_success "Binaries validated"
    return 0
}

# =============================================================================
# Test result utilities
# =============================================================================

create_test_summary() {
    local total_tests="$1"
    local passed_tests="$2"
    local failed_tests="$3"
    local test_name="$4"
    
    echo >&3
    log_info "Test Summary: $test_name" >&3
    log_info "Total tests: $total_tests" >&3
    log_info "Passed: $passed_tests" >&3
    log_info "Failed: $failed_tests" >&3
    
    if [[ $failed_tests -eq 0 ]]; then
        log_success "All tests passed!" >&3
        return 0
    else
        log_error "$failed_tests test(s) failed" >&3
        return 1
    fi
}

# Export functions for use in tests
export -f log_debug log_info log_warn log_error log_success log_test log_step log_check
export -f execute_platform_command execute_wine_command execute_binary_command
export -f validate_command_exists validate_file_exists validate_directory_exists
export -f count_files_by_pattern validate_file_format count_valid_files_by_format
export -f create_directory_if_needed create_test_summary
export -f validate_test_assets validate_binaries detect_binary_path
