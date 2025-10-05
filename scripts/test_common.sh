#!/bin/bash

# =============================================================================
# Common Test Utilities for art2img
# =============================================================================
# This script provides shared utilities for test scripts
# =============================================================================

# -----------------------------------------------------------------------------
# Configuration detection
# -----------------------------------------------------------------------------
detect_project_root() {
    local script_dir="$(cd "$(dirname "${BASH_SOURCE[1]}")" && pwd)"
    dirname "$script_dir"
}

detect_build_dir() {
    echo "${BUILD_DIR:-build}"
}

# -----------------------------------------------------------------------------
# Logging utilities
# -----------------------------------------------------------------------------
declare -A LOG_LEVELS=(
    ["DEBUG"]=0
    ["INFO"]=1
    ["WARN"]=2
    ["ERROR"]=3
    ["PASS"]=4
    ["TEST"]=5
    ["STEP"]=6
    ["CHECK"]=7
)

# Default log level
LOG_LEVEL="${LOG_LEVEL:-INFO}"

should_log() {
    local level="$1"
    local current_level="${LOG_LEVELS[$LOG_LEVEL]:-1}"
    local message_level="${LOG_LEVELS[$level]:-1}"
    
    [[ $message_level -ge $current_level ]]
}

log_message() {
    local level="$1"
    shift
    local message="$*"
    
    if should_log "$level"; then
        echo "[$level] $message"
    fi
}

log_debug() { log_message "DEBUG" "$@"; }
log_info() { log_message "INFO" "$@"; }
log_warn() { log_message "WARN" "$@"; }
log_error() { log_message "ERROR" "$@" >&2; }
log_success() { log_message "PASS" "$@"; }
log_test() { log_message "TEST" "$@"; }
log_step() { log_message "STEP" "$@"; }
log_check() { log_message "CHECK" "$@"; }

# -----------------------------------------------------------------------------
# System validation utilities
# -----------------------------------------------------------------------------
validate_command_exists() {
    local cmd="$1"
    local description="${2:-$cmd}"
    
    log_check "Validating $description"
    
    if command -v "$cmd" >/dev/null 2>&1; then
        local version
        case "$cmd" in
            "wine")
                version=$(wine --version 2>/dev/null || echo "unknown")
                ;;
            "gcc"|"g++"|"clang"|"clang++")
                version=$("$cmd" --version | head -n1)
                ;;
            "cmake")
                version=$(cmake --version | head -n1)
                ;;
            "x86_64-w64-mingw32-g++")
                version=$(x86_64-w64-mingw32-g++ --version | head -n1)
                ;;
            *)
                version="available"
                ;;
        esac
        log_success "$description found: $version"
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

# -----------------------------------------------------------------------------
# File utilities
# -----------------------------------------------------------------------------
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

# -----------------------------------------------------------------------------
# Execution utilities
# -----------------------------------------------------------------------------
execute_command() {
    local cmd="$1"
    local description="${2:-command}"
    local verbose="${3:-false}"
    local allow_failure="${4:-false}"
    
    log_test "Executing: $description"
    
    if [[ "$verbose" == "true" ]]; then
        log_debug "Command: $cmd"
    fi
    
    if eval "$cmd" >/dev/null 2>&1; then
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

execute_command_verbose() {
    local cmd="$1"
    local description="${2:-command}"
    local allow_failure="${3:-false}"
    
    log_test "Executing: $description"
    log_debug "Command: $cmd"
    
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

# -----------------------------------------------------------------------------
# Wine utilities
# -----------------------------------------------------------------------------
execute_wine_command() {
    local wine_cmd="$1"
    local description="${2:-Wine command}"
    local verbose="${3:-false}"
    local allow_failure="${4:-false}"

    local full_cmd="wine $wine_cmd"

    if [[ "$verbose" == "true" ]]; then
        execute_command_verbose "$full_cmd" "$description" "$allow_failure"
    else
        execute_command "$full_cmd" "$description" "false" "$allow_failure"
    fi
}

execute_wine_command_verbose() {
    local wine_cmd="$1"
    local description="${2:-Wine command}"
    local allow_failure="${3:-false}"

    execute_wine_command "$wine_cmd" "$description" "true" "$allow_failure"
}

validate_wine_installation() {
    validate_command_exists "wine" "Wine"
}

validate_wine_functionality() {
    log_check "Testing Wine functionality"
    
    if wine wineboot --init >/dev/null 2>&1; then
        log_success "Wine functionality validated"
        return 0
    else
        log_error "Wine initialization failed"
        return 1
    fi
}

# -----------------------------------------------------------------------------
# Test result utilities
# -----------------------------------------------------------------------------
create_test_summary() {
    local total_tests="$1"
    local passed_tests="$2"
    local failed_tests="$3"
    local test_name="$4"
    
    echo
    log_info "Test Summary: $test_name"
    log_info "Total tests: $total_tests"
    log_info "Passed: $passed_tests"
    log_info "Failed: $failed_tests"
    
    if [[ $failed_tests -eq 0 ]]; then
        log_success "All tests passed!"
        return 0
    else
        log_error "$failed_tests test(s) failed"
        return 1
    fi
}

# -----------------------------------------------------------------------------
# Configuration utilities
# -----------------------------------------------------------------------------
load_configuration() {
    local config_file="${1:-}"
    
    if [[ -n "$config_file" && -f "$config_file" ]]; then
        log_debug "Loading configuration from: $config_file"
        # shellcheck source=/dev/null
        source "$config_file"
    fi
}

# -----------------------------------------------------------------------------
# Cleanup utilities
# -----------------------------------------------------------------------------
cleanup_on_exit() {
    local cleanup_function="$1"
    
    # Set up trap to call cleanup function on exit
    trap "$cleanup_function" EXIT INT TERM
}

# -----------------------------------------------------------------------------
# Version comparison utilities
# -----------------------------------------------------------------------------
version_compare() {
    local version1="$1"
    local operator="$2"
    local version2="$3"
    
    # Simple version comparison using sort -V
    local result
    result=$(printf '%s\n%s\n' "$version1" "$version2" | sort -V | head -n1)
    
    case "$operator" in
        "="|"==")
            [[ "$version1" == "$version2" ]]
            ;;
        "!=")
            [[ "$version1" != "$version2" ]]
            ;;
        "<")
            [[ "$result" == "$version1" && "$version1" != "$version2" ]]
            ;;
        "<=")
            [[ "$result" == "$version1" ]]
            ;;
        ">")
            [[ "$result" == "$version2" && "$version1" != "$version2" ]]
            ;;
        ">=")
            [[ "$result" == "$version2" ]]
            ;;
        *)
            log_error "Invalid comparison operator: $operator"
            return 1
            ;;
    esac
}

# -----------------------------------------------------------------------------
# Export functions for sourcing
# -----------------------------------------------------------------------------
if [[ "${BASH_SOURCE[0]}" != "${0}" ]]; then
    # Script is being sourced
    export -f log_debug log_info log_warn log_error log_success log_test log_step log_check
    export -f validate_command_exists validate_file_exists validate_directory_exists
    export -f count_files_by_pattern validate_file_format count_valid_files_by_format
    export -f create_directory_if_needed execute_command execute_command_verbose
    export -f execute_wine_command execute_wine_command_verbose
    export -f validate_wine_installation validate_wine_functionality
    export -f create_test_summary load_configuration cleanup_on_exit version_compare
    export -f detect_project_root detect_build_dir
fi
