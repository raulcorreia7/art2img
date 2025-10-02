#!/bin/bash
set -e

echo "Building art2img for all platforms..."

# Build Linux binaries
echo "Building Linux x86_64 binaries..."
make clean
make linux

# Build Windows binaries if cross-compiler available
if command -v x86_64-w64-mingw32-g++ >/dev/null 2>&1; then
    echo "Building Windows x86_64 binaries..."
    make windows
else
    echo "⚠ Windows x86_64 cross-compiler not available, skipping"
fi

echo "Build complete!"
echo "Available binaries:"
ls -la bin/ 2>/dev/null || echo "No binaries found"

# Verify all available binaries
echo "Verifying binary architectures..."
make verify

# Test Linux binaries
echo "Testing Linux x86_64 binaries..."
make test

echo "All builds completed successfully!"