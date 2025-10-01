#!/bin/bash
set -e

echo "=== CI Test Suite for art2img ==="

# Test basic functionality
echo "1. Testing basic functionality..."
./bin/art2img --version
./bin/art2img --help

# Test extraction with different formats
echo "2. Testing PNG extraction..."
mkdir -p tests/output/ci_png
./bin/art2img -o tests/output/ci_png -f png -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART

echo "3. Testing TGA extraction..."
mkdir -p tests/output/ci_tga
./bin/art2img -o tests/output/ci_tga -f tga -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART

echo "4. Testing transparency options..."
mkdir -p tests/output/ci_transparency
./bin/art2img -o tests/output/ci_transparency -f png -p tests/assets/PALETTE.DAT tests/assets/TILES000.ART

mkdir -p tests/output/ci_no_transparency
./bin/art2img -o tests/output/ci_no_transparency -f png -p tests/assets/PALETTE.DAT -N tests/assets/TILES000.ART

# Verify output files exist
echo "5. Verifying output files..."
PNG_COUNT=$(find tests/output/ci_png -name "*.png" | wc -l)
TGA_COUNT=$(find tests/output/ci_tga -name "*.tga" | wc -l)
TRANSP_COUNT=$(find tests/output/ci_transparency -name "*.png" | wc -l)
NO_TRANSP_COUNT=$(find tests/output/ci_no_transparency -name "*.png" | wc -l)

echo "PNG files created: $PNG_COUNT"
echo "TGA files created: $TGA_COUNT"
echo "Transparent PNG files created: $TRANSP_COUNT"
echo "Non-transparent PNG files created: $NO_TRANSP_COUNT"

# Check if we have the expected number of files (256 tiles from TILES000.ART)
if [ "$PNG_COUNT" -eq "256" ] && [ "$TGA_COUNT" -eq "256" ] && [ "$TRANSP_COUNT" -eq "256" ] && [ "$NO_TRANSP_COUNT" -eq "256" ]; then
    echo "✅ All tests passed!"
    exit 0
else
    echo "❌ Test failed: Unexpected number of output files"
    exit 1
fi