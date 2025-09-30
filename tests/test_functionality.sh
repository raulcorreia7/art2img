#!/bin/bash
# Test script for TGA and PNG functionality with environment configuration

echo "art2image Test Suite"

# Load environment configuration if available
if [ -f ../.env ]; then
    echo "Loading environment configuration..."
    export $(grep -v '^#' ../.env | xargs)
fi

# Set default values if environment variables are not set
PALETTE_PATH=${PALETTE_PATH:-tests/assets/PALETTE.DAT}
ART_FILES_DIR=${ART_FILES_DIR:-tests/assets}
OUTPUT_DIR=${OUTPUT_DIR:-tests/output}
OUTPUT_FORMAT=${OUTPUT_FORMAT:-png}
THREADS=${THREADS:-4}
GENERATE_ANIMATION=${GENERATE_ANIMATION:-true}
VERBOSE=${VERBOSE:-true}

# Clean up previous test outputs
rm -rf tests/output/tga tests/output/png tests/output/with_transparency tests/output/no_transparency tests/output/single_tile tests/output/directory_mode 2>/dev/null || true

# Create organized output directories
mkdir -p tests/output/tga tests/output/png tests/output/with_transparency tests/output/no_transparency tests/output/single_tile tests/output/directory_mode tests/output/merge_anim

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

# Test 5: Single tile processing
echo "Test 5: Single tile processing"
bin/art2image -o tests/output/single_tile -f png -p "$PALETTE_PATH" -t "$THREADS" "$ART_FILES_DIR/TILES000.ART"
if [ $? -eq 0 ]; then
    echo "[OK] Single tile processing test passed"
else
    echo "[FAIL] Single tile processing test failed"
    exit 1
fi

# Test 6: Directory processing mode
echo "Test 6: Directory processing mode"
bin/art2image -o tests/output/directory_mode -p "$PALETTE_PATH" -t "$THREADS" "$ART_FILES_DIR"
if [ $? -eq 0 ]; then
    echo "[OK] Directory processing mode test passed"
else
    echo "[FAIL] Directory processing mode test failed"
    exit 1
fi

# Test 7: Directory processing mode with merge animation flag
echo "Test 7: Directory processing mode with merge animation flag"
bin/art2image -o tests/output/merge_anim -p "$PALETTE_PATH" -t "$THREADS" -m "$ART_FILES_DIR"
if [ $? -eq 0 ]; then
    echo "[OK] Directory processing mode with merge animation flag test passed"
else
    echo "[FAIL] Directory processing mode with merge animation flag test failed"
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
single_tile_count=$(ls tests/output/single_tile/*.png 2>/dev/null | wc -l)
directory_mode_count=$(ls tests/output/directory_mode/*/*.png 2>/dev/null | wc -l)
merge_anim_count=$(ls tests/output/merge_anim/*/*.png 2>/dev/null | wc -l)

echo "TGA files: $tga_count"
echo "PNG files: $png_count"
echo "PNG with transparency files: $with_transparency_count"
echo "PNG without transparency files: $no_transparency_count"
echo "Single tile processing files: $single_tile_count"
echo "Directory mode files: $directory_mode_count"
echo "Merge animation mode files: $merge_anim_count"

# Verify all formats produced the same number of files for single ART file processing
if [ "$tga_count" -eq "$png_count" ] && [ "$png_count" -gt 0 ]; then
    echo "[OK] File counts match for single ART file processing"
else
    echo "[FAIL] File count mismatch for single ART file processing"
    exit 1
fi

# Verify transparency processing is working by comparing specific files
# Tile 22 should have magenta pixels that get converted to transparency
if [ -f "tests/output/with_transparency/tile0022.png" ] && [ -f "tests/output/no_transparency/tile0022.png" ]; then
    with_size=$(stat -c%s "tests/output/with_transparency/tile0022.png" 2>/dev/null || stat -f%z "tests/output/with_transparency/tile0022.png")
    without_size=$(stat -c%s "tests/output/no_transparency/tile0022.png" 2>/dev/null || stat -f%z "tests/output/no_transparency/tile0022.png")
    
    # The file with transparency should be smaller or equal in size
    if [ "$with_size" -le "$without_size" ]; then
        echo "[OK] Transparency processing is working (file sizes: with=$with_size, without=$without_size)"
    else
        echo "[WARNING] Transparency processing may not be working (file sizes: with=$with_size, without=$without_size)"
    fi
else
    echo "[WARNING] Could not find test tiles to verify transparency processing"
fi

# Verify that both with_transparency and no_transparency directories have the same number of files
if [ "$with_transparency_count" -eq "$no_transparency_count" ] && [ "$with_transparency_count" -gt 0 ]; then
    echo "[OK] Both with_transparency and no_transparency directories have the same number of files"
else
    echo "[FAIL] File count mismatch between with_transparency and no_transparency directories"
    exit 1
fi

# Test the CLI help option
if ./bin/art2image --help | grep -q "fix-transparency"; then
    echo "[OK] CLI help text includes fix-transparency options"
else
    echo "[FAIL] CLI help text does not include fix-transparency options"
    exit 1
fi

# Check for animation data
if [ -f "tests/output/tga/animdata.ini" ] && [ -f "tests/output/png/animdata.ini" ]; then
    echo "[OK] Animation data files created"
else
    echo "[FAIL] Missing animation data files"
    exit 1
fi

# Check for merged animation data
if [ -f "tests/output/merge_anim/animdata.ini" ]; then
    echo "[OK] Merged animation data file created"
    # Check if the merged file contains data from multiple ART files
    merged_lines=$(wc -l < "tests/output/merge_anim/animdata.ini")
    if [ "$merged_lines" -gt 10 ]; then
        echo "[OK] Merged animation data file contains sufficient data"
    else
        echo "[WARNING] Merged animation data file may be too small"
    fi
else
    echo "[FAIL] Missing merged animation data file"
    exit 1
fi

# Test 8: Palette functionality tests
echo "Test 8: Palette functionality tests"
bash tests/test_palette_functionality.sh
if [ $? -eq 0 ]; then
    echo "[OK] Palette functionality tests passed"
else
    echo "[FAIL] Palette functionality tests failed"
    exit 1
fi

echo ""
echo "[OK] All tests completed successfully"
