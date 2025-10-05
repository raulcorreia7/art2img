#!/bin/bash

# =============================================================================
# Script to run all Bats tests for art2img
# =============================================================================

set -e

# Find project root
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
LOCAL_BATS_DIR="$PROJECT_ROOT/.bin/bats"
LOCAL_BATS_BIN="$LOCAL_BATS_DIR/bin/bats"

# Set platform from environment or detect automatically
PLATFORM="${PLATFORM:-auto}"

# Detect platform if set to auto
if [[ "$PLATFORM" == "auto" ]]; then
    if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]] || [[ "$RUNNER_OS" == "Windows" ]]; then
        PLATFORM="windows"
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        PLATFORM="macos"
    else
        PLATFORM="linux"
    fi
fi

echo "Detected platform: $PLATFORM"

# For Windows, we might need to check different availability of test framework
if [[ "$PLATFORM" == "windows" ]]; then
    # Check if we're in a Windows environment with Git Bash or WSL
    # Try to use bats if available, otherwise warn about limitations
    echo "Running on Windows platform"
    # Check if bats is available in Windows environment (e.g., Git Bash)
    if command -v bats >/dev/null 2>&1; then
        BATS_CMD="bats"
        echo "Bats found in Windows environment"
    else
        echo "Warning: Bats not available in Windows environment"
        echo "Skipping Bats tests on Windows platform"
        exit 0
    fi
else
    # Check if local bats exists, otherwise try system bats
    if [[ -f "$LOCAL_BATS_BIN" ]]; then
        BATS_CMD="$LOCAL_BATS_BIN"
    elif command -v bats >/dev/null 2>&1; then
        BATS_CMD="bats"
    else
        echo "Error: bats is not installed locally or in PATH"
        echo "Install locally using: ./scripts/install_bats_local.sh"
        echo "Or install system-wide: git clone https://github.com/bats-core/bats-core.git && cd bats-core && sudo ./install.sh /usr/local"
        exit 1
    fi
fi

# Check if build directory exists
if [[ ! -d "$PROJECT_ROOT/build" ]] && [[ ! -d "$PROJECT_ROOT/build/linux-release" ]] && [[ ! -d "$PROJECT_ROOT/build/windows-release" ]] && [[ ! -d "$PROJECT_ROOT/build/windows-x86-release" ]]; then
    echo "Error: Build directory not found. Please build the project first."
    echo "Run: cd $PROJECT_ROOT && make build"
    exit 1
fi

echo "Running Bats tests for art2img..."
echo "Project root: $PROJECT_ROOT"
echo "Test directory: $PROJECT_ROOT/tests/bats"
echo "Bats command: $BATS_CMD"
echo "Platform: $PLATFORM"
echo ""

# Run all bats tests from the new location with pretty output and verbose run
# Set TERM to avoid tput errors in CI environments
TERM=dumb "$BATS_CMD" "$PROJECT_ROOT/tests/bats" -p --verbose-run

echo ""
echo "Bats tests completed."