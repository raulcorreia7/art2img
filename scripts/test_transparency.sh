#!/bin/bash
# Test script for transparency behavior validation

set -e

echo "=== Transparency Test Script ==="

# Create output directories
OUTPUT_DIR="test_output/transparency"
mkdir -p "$OUTPUT_DIR"

# Build the project if needed
if [ ! -f "build/bin/art2img" ]; then
    echo "Building project..."
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release
    make -j$(nproc)
    cd ..
fi

# Test with transparency enabled (default)
echo "1. Testing with transparency enabled (default)..."
./build/bin/art2img -f png -o "$OUTPUT_DIR/with_transparency" tests/assets/TILES000.ART

# Test with transparency disabled
echo "2. Testing with transparency disabled..."
./build/bin/art2img -f png -o "$OUTPUT_DIR/no_transparency" -N tests/assets/TILES000.ART

# Test with different formats
echo "3. Testing BMP format with transparency..."
./build/bin/art2img -f bmp -o "$OUTPUT_DIR/bmp_transparency" tests/assets/TILES000.ART

echo "4. Testing BMP format without transparency..."
./build/bin/art2img -f bmp -o "$OUTPUT_DIR/bmp_no_transparency" -N tests/assets/TILES000.ART

# Test with TGA format (no transparency support)
echo "5. Testing TGA format (no transparency support)..."
./build/bin/art2img -f tga -o "$OUTPUT_DIR/tga" tests/assets/TILES000.ART

# Report results
echo ""
echo "=== Test Results ==="
echo "Files with transparency: $(find "$OUTPUT_DIR/with_transparency" -name "*.png" | wc -l)"
echo "Files without transparency: $(find "$OUTPUT_DIR/no_transparency" -name "*.png" | wc -l)"
echo "BMP files with transparency: $(find "$OUTPUT_DIR/bmp_transparency" -name "*.bmp" | wc -l)"
echo "BMP files without transparency: $(find "$OUTPUT_DIR/bmp_no_transparency" -name "*.bmp" | wc -l)"
echo "TGA files: $(find "$OUTPUT_DIR/tga" -name "*.tga" | wc -l)"

echo ""
echo "Test files generated in: $OUTPUT_DIR"
echo "You can manually inspect the files to verify transparency behavior."

# Run the unit tests as well
echo ""
echo "6. Running unit tests for transparency..."
cd build
ctest -R transparency --output-on-failure
cd ..

echo ""
echo "=== Transparency testing complete ==="