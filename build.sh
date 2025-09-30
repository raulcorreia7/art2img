#!/bin/bash
set -e

echo "Building art2image for all platforms..."

# Build Linux binaries
echo "Building Linux binaries..."
make clean
make

# Build Windows binaries
echo "Building Windows binaries..."
make windows

echo "Build complete!"
echo "Linux binaries: bin/art2image, bin/art_diagnostic"
echo "Windows binaries: bin/art2image.exe, bin/art_diagnostic.exe"

# Test Linux binaries
echo "Testing Linux binaries..."
make test

echo "All builds completed successfully!"