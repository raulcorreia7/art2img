#!/bin/bash
set -e

echo "Building art2img with CMake..."

# Detect number of processors for parallel builds
NPROCS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)

# Clean previous build if it exists
if [ -d "build" ]; then
    echo "Cleaning previous build..."
    rm -rf build
fi

# Create build directory
echo "Creating build directory..."
mkdir -p build
cd build

# Configure with CMake
echo "Configuring with CMake..."
cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON -DBUILD_TESTS=ON

# Build with parallel compilation
echo "Building with ${NPROCS} parallel jobs..."
cmake --build . --parallel ${NPROCS}

echo "Build complete!"
echo "Available binaries:"
ls -la bin/ 2>/dev/null || echo "No binaries found"

# Verify binary architectures
echo "Verifying binary architectures..."
if command -v file >/dev/null 2>&1; then
    for bin in bin/art2img bin/art2img_test; do
        if [ -f "$bin" ]; then
            echo "- $(basename $bin): $(file -b $bin)"
        fi
    done
else
    echo "⚠ 'file' command not available, skipping architecture verification"
fi

# Run tests
echo "Running tests..."
ctest --output-on-failure

# Build Windows binaries if cross-compiler available
if command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
    echo "Building Windows x86_64 binaries..."

    # Clean Windows build if it exists
    if [ -d "build-win" ]; then
        rm -rf build-win
    fi

    mkdir -p build-win
    cd build-win

    # Configure Windows cross-compilation
    cmake .. -DCMAKE_BUILD_TYPE=Release -DBUILD_CLI=ON -DBUILD_TESTS=OFF \
             -DCMAKE_TOOLCHAIN_FILE=../cmake/windows-toolchain.cmake

    cmake --build . --parallel ${NPROCS}

    echo "Windows build complete!"
    cd ..
else
    echo "⚠ Windows x86_64 cross-compiler not available, skipping"
fi

echo "All builds completed successfully!"