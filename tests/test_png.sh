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
rm -rf tests/output/tga tests/output/png tests/output/with_transparency tests/output/no_transparency 2>/dev/null || true

# Create organized output directories
mkdir -p tests/output/tga tests/output/png tests/output/with_transparency tests/output/no_transparency

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

# Test 3: PNG format with magenta transparency fix (default)
echo "Test 3: PNG format with magenta transparency fix (default)"
bin/art2image -o tests/output/with_transparency -f png -p "$PALETTE_PATH" -t "$THREADS" "$ART_FILES_DIR/TILES000.ART"
if [ $? -eq 0 ]; then
    echo "[OK] PNG with transparency test passed"
else
    echo "[FAIL] PNG with transparency test failed"
    exit 1
fi

# Test 4: PNG format without magenta transparency fix
echo "Test 4: PNG format without magenta transparency fix"
bin/art2image -o tests/output/no_transparency -f png -p "$PALETTE_PATH" -t "$THREADS" -N "$ART_FILES_DIR/TILES000.ART"
if [ $? -eq 0 ]; then
    echo "[OK] PNG without transparency test passed"
else
    echo "[FAIL] PNG without transparency test failed"
    exit 1
fi

# Validate outputs
echo ""
echo "Output validation:"

# Check file counts and basic properties
tga_count=$(ls tests/output/tga/*.tga 2>/dev/null | wc -l)
png_count=$(ls tests/output/png/*.png 2>/dev/null | wc -l)
with_transparency_count=$(ls tests/output/with_transparency/*.png 2>/dev/null | wc -l)
no_transparency_count=$(ls tests/output/no_transparency/*.png 2>/dev/null | wc -l)

echo "TGA files: $tga_count"
echo "PNG files: $png_count"
echo "PNG with transparency files: $with_transparency_count"
echo "PNG without transparency files: $no_transparency_count"

# Verify all formats produced the same number of files
if [ "$tga_count" -eq "$png_count" ] && [ "$png_count" -eq "$with_transparency_count" ] && [ "$with_transparency_count" -eq "$no_transparency_count" ] && [ "$tga_count" -gt 0 ]; then
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
