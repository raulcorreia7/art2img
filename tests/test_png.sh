#!/bin/bash
# Test script for TGA and PNG functionality with environment configuration

echo "art2image Test Suite"

# Load environment configuration if available
if [ -f ../.env ]; then
    echo "Loading environment configuration..."
    export $(grep -v '^#' ../.env | xargs)
fi

# Set default values if environment variables are not set
PALETTE_PATH=${PALETTE_PATH:-tests/assets/duke3d/PALETTE.DAT}
ART_FILES_DIR=${ART_FILES_DIR:-tests/assets/duke3d}
OUTPUT_DIR=${OUTPUT_DIR:-tests/output}
OUTPUT_FORMAT=${OUTPUT_FORMAT:-png}
THREADS=${THREADS:-4}
GENERATE_ANIMATION=${GENERATE_ANIMATION:-true}
VERBOSE=${VERBOSE:-true}

# Clean up previous test outputs
rm -rf tests/output/tga tests/output/png 2>/dev/null || true

# Create organized output directories
mkdir -p tests/output/tga tests/output/png

# Test 1: TGA format
echo "Test 1: TGA format"
bin/art2image -o tests/output/tga -f tga -p "$PALETTE_PATH" -t "$THREADS" "$ART_FILES_DIR/TILES000.ART"
if [ $? -eq 0 ]; then
    echo "[OK] TGA test passed"
else
    echo "[FAIL] TGA test failed"
    exit 1
fi

# Test 2: PNG format
echo "Test 2: PNG format"
bin/art2image -o tests/output/png -f png -p "$PALETTE_PATH" -t "$THREADS" "$ART_FILES_DIR/TILES000.ART"
if [ $? -eq 0 ]; then
    echo "[OK] PNG test passed"
else
    echo "[FAIL] PNG test failed"
    exit 1
fi

# Validate outputs
echo ""
echo "Output validation:"

# Check file counts and basic properties
tga_count=$(ls tests/output/tga/*.tga 2>/dev/null | wc -l)
png_count=$(ls tests/output/png/*.png 2>/dev/null | wc -l)

echo "TGA files: $tga_count"
echo "PNG files: $png_count"

# Verify both formats produced the same number of files
if [ "$tga_count" -eq "$png_count" ] && [ "$tga_count" -gt 0 ]; then
    echo "[OK] File counts match"
else
    echo "[FAIL] File count mismatch"
    exit 1
fi

# Check for animation data
if [ -f "tests/output/tga/animdata.ini" ] && [ -f "tests/output/png/animdata.ini" ]; then
    echo "[OK] Animation data files created"
else
    echo "[FAIL] Missing animation data files"
    exit 1
fi

echo ""
echo "[OK] All tests completed successfully"
