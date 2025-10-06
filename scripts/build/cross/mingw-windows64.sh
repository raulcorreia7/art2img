#!/bin/bash

# Cross-compilation script for Linux to Windows 64-bit
# Uses MinGW-w64 toolchain

set -e

# Source common utilities
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
COMMON_DIR="$(dirname "$(dirname "$SCRIPT_DIR")")/build/common.sh"

if [[ -f "$COMMON_DIR" ]]; then
    source "$COMMON_DIR"
else
    echo "Error: Could not find common build utilities at $COMMON_DIR"
    exit 1
fi

# Validate dependencies
validate_dependencies

# Check for required cross-compilers
if ! command_exists x86_64-w64-mingw32-gcc || ! command_exists x86_64-w64-mingw32-g++; then
    error_exit "Required MinGW-w64 cross-compilers (x86_64-w64-mingw32-gcc/g++) not found. Please install mingw-w64 package."
fi

# Default parameters
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"
WINDOWS_X64_BUILD_DIR="$BUILD_DIR"

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            WINDOWS_X64_BUILD_DIR="$BUILD_DIR"
            shift 2
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --clean)
            clean_build "$WINDOWS_X64_BUILD_DIR"
            log_info "Cleaned Windows x64 build directory"
            exit 0
            ;;
        *)
            log_warn "Unknown option: $1"
            shift
            ;;
    esac
done

# Update Windows x64 build directory based on potentially updated BUILD_DIR
WINDOWS_X64_BUILD_DIR="$BUILD_DIR"

# Create build directory
create_build_dir "$WINDOWS_X64_BUILD_DIR"

# Configure CMake for Windows x64 cross-compilation
log_info "Configuring CMake for Windows x64 cross-compilation..."
# Calculate the relative path from build directory to project root
SOURCE_DIR_RELATIVE="$(realpath --relative-to="$WINDOWS_X64_BUILD_DIR" .)"
configure_cmake "$SOURCE_DIR_RELATIVE" "$WINDOWS_X64_BUILD_DIR" \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DCMAKE_TOOLCHAIN_FILE="$(realpath --relative-to="$WINDOWS_X64_BUILD_DIR" cmake/windows-toolchain.cmake)" \
    -DBUILD_SHARED_LIBS=OFF

# Build the project
log_info "Building Windows x64 binaries..."
build_project "$WINDOWS_X64_BUILD_DIR"

log_info "Windows x64 cross-compilation completed successfully!"
echo "Binaries are available in: $WINDOWS_X64_BUILD_DIR/bin/"