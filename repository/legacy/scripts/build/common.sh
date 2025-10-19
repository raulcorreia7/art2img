#!/bin/bash

# Common build utilities and functions for art2img project

# Exit on error
set -e

# Error handling function
error_exit() {
    echo "Error: $1" >&2
    exit 1
}

# Logging functions
log_info() {
    echo "INFO: $1"
}

log_warn() {
    echo "WARN: $1"
}

log_error() {
    echo "ERROR: $1" >&2
}

# Check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Validate that required tools are available
validate_dependencies() {
    local missing_deps=()
    
    for tool in cmake make; do
        if ! command_exists "$tool"; then
            missing_deps+=("$tool")
        fi
    done
    
    if [[ ${#missing_deps[@]} -gt 0 ]]; then
        error_exit "Missing required tools: ${missing_deps[*]}"
    fi
}

# Default build directory
BUILD_DIR="${BUILD_DIR:-build}"

# Default jobs for parallel builds
JOBS="${JOBS:-$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)}"

# Project version
VERSION=$(cmake -P cmake/print_version.cmake 2>/dev/null || echo "unknown")

# Default CMake flags
CMAKE_BASE_FLAGS="${CMAKE_BASE_FLAGS:--DBUILD_TESTS=ON}"
CMAKE_RELEASE_FLAGS="${CMAKE_RELEASE_FLAGS:-$CMAKE_BASE_FLAGS -DCMAKE_BUILD_TYPE=Release -DBUILD_DIAGNOSTIC=ON -DBUILD_SHARED_LIBS=OFF}"

# Function to create build directory
create_build_dir() {
    local build_path="$1"
    mkdir -p "$build_path" || error_exit "Failed to create build directory: $build_path"
}

# Function to run cmake configuration
configure_cmake() {
    local source_dir="$1"
    local build_dir="$2"
    shift 2
    local extra_flags=("$@")
    
    (cd "$build_dir" && cmake "$source_dir" "${extra_flags[@]}") || error_exit "CMake configuration failed"
}

# Function to build using cmake
build_project() {
    local build_dir="$1"
    shift
    local extra_args=("$@")
    
    cmake --build "$build_dir" --parallel "$JOBS" "${extra_args[@]}" || error_exit "Build failed"
}

# Function to run tests
run_tests() {
    local build_dir="$1"
    (cd "$build_dir" && ctest --output-on-failure) || error_exit "Tests failed"
}

# Function to install the project
install_project() {
    local build_dir="$1"
    local prefix="${2:-/usr/local}"
    
    (cd "$build_dir" && cmake --install . --prefix "$prefix") || error_exit "Installation failed"
}

# Function to run code formatting
run_format() {
    local build_dir="$1"
    (cd "$build_dir" && cmake --build . --target clang-format) || error_exit "Formatting failed"
}

# Function to run format check
run_format_check() {
    local build_dir="$1"
    (cd "$build_dir" && cmake --build . --target clang-format-dry-run) || error_exit "Format check failed"
}

# Function to run linting
run_lint() {
    local build_dir="$1"
    (cd "$build_dir" && cmake --build . --target clang-tidy) || error_exit "Linting failed"
}

# Function to clean build directory
clean_build() {
    local build_dir="$1"
    if [[ -d "$build_dir" ]]; then
        rm -rf "$build_dir" || error_exit "Failed to clean build directory: $build_dir"
    fi
}