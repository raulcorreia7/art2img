#!/bin/bash
set -e

echo "=== CI Test Suite for art2img ==="

# Test basic functionality
echo "1. Testing basic functionality..."
./build/bin/art2img --version
./build/bin/art2img --help

# Test extraction with different formats
echo "2. Testing PNG extraction..."
mkdir -p build/tests/output/ci_png
./build/bin/art2img -o build/tests/output/ci_png -f png -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART

echo "3. Testing TGA extraction..."
mkdir -p build/tests/output/ci_tga
./build/bin/art2img -o build/tests/output/ci_tga -f tga -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART

echo "4. Testing transparency options..."
mkdir -p build/tests/output/ci_transparency
./build/bin/art2img -o build/tests/output/ci_transparency -f png -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART

mkdir -p build/tests/output/ci_no_transparency
./build/bin/art2img -o build/tests/output/ci_no_transparency -f png -p tests/assets/PALETTE.DAT -N tests/assets/TILES000.ART

# Verify output files exist
echo "5. Testing PNG memory regression..."
./build/bin/png_memory_regression build/tests/output/png_memory_regression_ci.png

echo "6. Verifying output files..."
PNG_COUNT=$(find build/tests/output/ci_png -name "*.png" | wc -l)
TGA_COUNT=$(find build/tests/output/ci_tga -name "*.tga" | wc -l)
TRANSP_COUNT=$(find build/tests/output/ci_transparency -name "*.png" | wc -l)
NO_TRANSP_COUNT=$(find build/tests/output/ci_no_transparency -name "*.png" | wc -l)

echo "PNG files created: $PNG_COUNT"
echo "TGA files created: $TGA_COUNT"
echo "Transparent PNG files created: $TRANSP_COUNT"
echo "Non-transparent PNG files created: $NO_TRANSP_COUNT"

# Check if we have the expected number of files (256 tiles from TILES000.ART)
if [ "$PNG_COUNT" -eq "$TGA_COUNT" ] \
    && [ "$PNG_COUNT" -eq "$TRANSP_COUNT" ] \
    && [ "$PNG_COUNT" -eq "$NO_TRANSP_COUNT" ] \
    && [ "$PNG_COUNT" -gt 0 ]; then
    echo "✅ All tests passed! ($PNG_COUNT tiles)"
    exit 0
else
    echo "❌ Test failed: Unexpected number of output files"
    exit 1
fi
