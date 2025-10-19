#!/bin/bash

# =============================================================================
# Test Windows Build Script for art2img
# =============================================================================
# This script tests Windows builds using Wine on Linux
# Usage: ./scripts/test_windows.sh [build_dir] [binary_dir]
# =============================================================================

set -e

# Parse arguments
BUILD_DIR="${1:-build/windows}"
BINARY_DIR="${2:-$BUILD_DIR}"

find_binary() {
    local search_root="$1"
    local basename="$2"

    if [[ ! -d "$search_root" ]]; then
        return 1
    fi

    mapfile -t matches < <(find "$search_root" -type f \( -name "$basename" -o -name "$basename.exe" \) 2>/dev/null)

    if [[ ${#matches[@]} -gt 0 ]]; then
        printf '%s\n' "${matches[0]}"
        return 0
    fi

    return 1
}

echo "Testing Windows build..."
echo "Build directory: $BUILD_DIR"
echo "Binary directory: $BINARY_DIR"

# Find project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Check if Wine is available
if ! command -v wine >/dev/null 2>&1; then
    echo "Error: Wine is not installed but is required for testing Windows builds"
    echo "Please install Wine to test Windows builds on Linux"
    exit 1
fi

# Locate Windows binaries (supports single and multi-config generators)
BIN_ROOT="$BINARY_DIR/bin"
MAIN_BIN="$(find_binary "$BIN_ROOT" "art2img")"
TEST_BIN="$(find_binary "$BIN_ROOT" "art2img_tests")"

if [[ -z "$MAIN_BIN" ]]; then
    echo "Error: Windows executable not found under $BIN_ROOT"
    exit 1
fi

if [[ -z "$TEST_BIN" ]]; then
    echo "Error: Windows test executable not found under $BIN_ROOT"
    exit 1
fi

echo "Windows binaries found, proceeding with tests..."

# Set platform for tests
export PLATFORM="windows"

# Run Windows executables through Wine
echo "Testing Windows executable version..."
if wine "$MAIN_BIN" --version; then
    echo "✓ Windows executable version check passed"
else
    echo "✗ Windows executable version check failed"
    exit 1
fi

echo "Testing Windows executable help..."
if wine "$MAIN_BIN" --help; then
    echo "✓ Windows executable help check passed"
else
    echo "✗ Windows executable help check failed"
    exit 1
fi

echo "Run unit tests on Windows binary..."
if wine "$TEST_BIN" --no-colors; then
    echo "✓ Windows unit tests passed"
else
    echo "✗ Windows unit tests failed"
    exit 1
fi

echo "All Windows build tests completed successfully!"
