#!/bin/bash

# Native build script for Linux
# Builds the project for the current Linux system

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

# Default parameters
BUILD_TYPE="${BUILD_TYPE:-Release}"
BUILD_DIR="${BUILD_DIR:-build}"
LINUX_BUILD_DIR="$BUILD_DIR/linux-x64"
USE_DIRECT_PATH=false

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -j|--jobs)
            JOBS="$2"
            shift 2
            ;;
        --build-dir)
            BUILD_DIR="$2"
            shift 2
            ;;
        --build-type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        --use-direct-path)
            USE_DIRECT_PATH=true
            LINUX_BUILD_DIR="$BUILD_DIR"
            shift
            ;;
        --clean)
            clean_build "$LINUX_BUILD_DIR"
            log_info "Cleaned Linux build directory"
            exit 0
            ;;
        *)
            log_warn "Unknown option: $1"
            shift
            ;;
    esac
done

# Update Linux build directory based on potentially updated BUILD_DIR
# Unless we're using direct path mode
if [[ "$USE_DIRECT_PATH" == false ]]; then
    LINUX_BUILD_DIR="$BUILD_DIR/linux-x64"
else
    LINUX_BUILD_DIR="$BUILD_DIR"
fi

# Create build directory
create_build_dir "$LINUX_BUILD_DIR"

# Configure CMake for Linux native build
log_info "Configuring CMake for Linux native build..."
# The script runs from the project root, and we want to build in the LINUX_BUILD_DIR
# The source code is 2 levels up from the LINUX_BUILD_DIR (relative to build/linux-x64, the source is ../..)
# since LINUX_BUILD_DIR is something like build/linux-x64, and the main CMakeLists.txt is at project root
if [[ "$USE_DIRECT_PATH" == true ]]; then
    # When using direct path, we're in the build directory directly
    configure_cmake "../.." "$LINUX_BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        $CMAKE_BASE_FLAGS
else
    # When using nested path, we're in build/linux-x64 directory
    configure_cmake "../.." "$LINUX_BUILD_DIR" \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        $CMAKE_BASE_FLAGS
fi

# Build the project
log_info "Building Linux binaries..."
build_project "$LINUX_BUILD_DIR"

log_info "Linux x64 native build completed successfully!"
echo "Binaries are available in: $LINUX_BUILD_DIR/bin/"