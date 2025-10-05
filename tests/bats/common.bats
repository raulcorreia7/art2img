#!/usr/bin/env bats

# =============================================================================
# Common Test Utilities for art2img BATS Tests
# =============================================================================

# Load BATS support libraries (if available)
# This ensures proper support for testing functionality
setup() {
    # Project root detection
    PROJECT_ROOT="$(cd "$(dirname "${BATS_TEST_FILENAME}")/../.." && pwd)"
    
    # Set default build directory
    : "${BUILD_DIR:=build}"
    
    # Detect platform
    if [[ -n "${1:-}" ]]; then
        PLATFORM="$1"
    elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || command -v wine >/dev/null 2>&1; then
        PLATFORM="windows"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        PLATFORM="macos"
    else
        PLATFORM="linux"
    fi
    
    # Set platform-specific paths
    case "$PLATFORM" in
        "windows")
            # Windows-specific paths (using Wine)
            BINARY_DIR="${BUILD_DIR}/bin"
            MAIN_BIN="${BINARY_DIR}/art2img.exe"
            TEST_BIN="${BINARY_DIR}/art2img_tests.exe"
            ;;
        *)
            # Unix-like systems (Linux, macOS)
            BINARY_DIR="${BUILD_DIR}/bin"
            MAIN_BIN="${BINARY_DIR}/art2img"
            TEST_BIN="${BINARY_DIR}/art2img_tests"
            ;;
    esac
    
    # Test assets directory
    TEST_ASSETS_DIR="${PROJECT_ROOT}/tests/assets"
    PALETTE_FILE="${TEST_ASSETS_DIR}/PALETTE.DAT"
    TEST_ART_FILE="${TEST_ASSETS_DIR}/TILES000.ART"
    
    # Setup output directories
    OUTPUT_DIR="${BATS_TEST_TMPDIR:-/tmp}/art2img_test_output_$$"
    mkdir -p "$OUTPUT_DIR"
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

# Colors for output (if tput is available)
if command -v tput >/dev/null 2>&1; then
    RED=$(tput setaf 1)
    GREEN=$(tput setaf 2)
    YELLOW=$(tput setaf 3)
    BLUE=$(tput setaf 4)
    BOLD=$(tput bold)
    RESET=$(tput sgr0)
else
    RED=""
    GREEN=""
    YELLOW=""
    BLUE=""
    BOLD=""
    RESET=""
fi

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
    
    # Use Wine for Windows binaries on non-Windows platforms
    if [[ "$PLATFORM" == "windows" ]]; then
        local full_cmd="wine $cmd"
    else
        local full_cmd="$cmd"
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
            execute_wine_command "$full_cmd" "$description" "$allow_failure"
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