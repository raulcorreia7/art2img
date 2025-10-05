#!/bin/bash

# ============================================================================= 
# Generic Test Script for art2img CI
# =============================================================================
# This script runs tests for art2img based on platform
# Usage: ./scripts/test_generic.sh [platform] [build_dir]
# =============================================================================

set -e

# Parse arguments
PLATFORM="${1:-linux}"
BUILD_DIR="${2:-build}"

echo "Running generic tests for platform: $PLATFORM"
echo "Build directory: $BUILD_DIR"

# Find project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Add bats script execution
BATS_SCRIPT="$SCRIPT_DIR/run_bats_tests.sh"

if [[ -f "$BATS_SCRIPT" ]]; then
    echo "Running Bats tests..."
    "$BATS_SCRIPT"
else
    echo "Error: Bats test runner script not found at $BATS_SCRIPT"
    echo "Please make sure the Bats testing framework is properly set up."
    exit 1
fi

echo "Generic tests completed successfully for platform: $PLATFORM"