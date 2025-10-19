#!/bin/bash
set -euo pipefail

# Build script for art2img project
BUILD_DIR="${1:-build}"
BUILD_TYPE="${2:-Release}"

echo "Building art2img in $BUILD_DIR with $BUILD_TYPE configuration"

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
cmake .. -DCMAKE_BUILD_TYPE="$BUILD_TYPE"

# Build
cmake --build . --config "$BUILD_TYPE"

echo "Build completed successfully!"
echo "Run tests with: cd $BUILD_DIR && ctest"