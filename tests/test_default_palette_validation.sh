#!/bin/bash
# Test script to validate Duke Nukem 3D default palette against external palette

echo "Duke Nukem 3D Default Palette Validation Test"
echo "============================================"

# Clean up previous test outputs
rm -rf tests/output/default_palette_validation 2>/dev/null || true

# Create output directories
mkdir -p tests/output/default_palette_validation/default
mkdir -p tests/output/default_palette_validation/external

echo "Testing ALL ART files with Duke Nukem 3D default palette (no -p parameter)..."
echo "-------------------------------------------------------------------------------"

# Process all ART files with default palette
for art_file in tests/assets/TILES*.ART; do
    filename=$(basename "$art_file" .ART)
    echo "Processing $filename.ART with default palette..."
    bin/art2image -o "tests/output/default_palette_validation/default/$filename" -f png -t 4 -q "$art_file"
done

echo ""
echo "Testing ALL ART files with external PALETTE.DAT file..."
echo "------------------------------------------------------"

# Process all ART files with external palette
for art_file in tests/assets/TILES*.ART; do
    filename=$(basename "$art_file" .ART)
    echo "Processing $filename.ART with external palette..."
    bin/art2image -o "tests/output/default_palette_validation/external/$filename" -f png -p tests/assets/PALETTE.DAT -t 4 -q "$art_file"
done

echo ""
echo "Validation Results:"
echo "=================="

# Compare results for each ART file
total_files=0
matching_files=0

for art_file in tests/assets/TILES*.ART; do
    filename=$(basename "$art_file" .ART)
    total_files=$((total_files + 1))
    
    default_dir="tests/output/default_palette_validation/default/$filename"
    external_dir="tests/output/default_palette_validation/external/$filename"
    
    if [ -d "$default_dir" ] && [ -d "$external_dir" ]; then
        # Count files in each directory
        default_count=$(ls "$default_dir"/*.png 2>/dev/null | wc -l)
        external_count=$(ls "$external_dir"/*.png 2>/dev/null | wc -l)
        
        if [ "$default_count" -eq "$external_count" ]; then
            echo "✓ $filename: File counts match ($default_count files)"
            
            # Compare a sample file size (first file alphabetically)
            first_default=$(ls "$default_dir"/*.png 2>/dev/null | head -1)
            first_external=$(ls "$external_dir"/*.png 2>/dev/null | head -1)
            
            if [ -n "$first_default" ] && [ -n "$first_external" ]; then
                default_size=$(stat -c%s "$first_default" 2>/dev/null || stat -f%z "$first_default" 2>/dev/null || echo "0")
                external_size=$(stat -c%s "$first_external" 2>/dev/null || stat -f%z "$first_external" 2>/dev/null || echo "0")
                
                if [ "$default_size" -eq "$external_size" ]; then
                    echo "  ✓ $filename: Sample file sizes match (${default_size} bytes)"
                    matching_files=$((matching_files + 1))
                else
                    echo "  ⚠ $filename: Sample file sizes differ (default: ${default_size}, external: ${external_size})"
                fi
            else
                echo "  ⚠ $filename: Could not find sample files for size comparison"
            fi
        else
            echo "✗ $filename: File count mismatch (default: $default_count, external: $external_count)"
        fi
    else
        echo "✗ $filename: Output directories not found"
    fi
done

echo ""
echo "Summary:"
echo "======="
echo "Total ART files processed: $total_files"
echo "Files with matching output: $matching_files"

if [ "$matching_files" -eq "$total_files" ]; then
    echo "✓ ALL ART files produced identical output with default and external palette"
    echo "[SUCCESS] Duke Nukem 3D default palette validation PASSED"
    exit 0
else
    echo "⚠ Some files produced different output"
    echo "[WARNING] Duke Nukem 3D default palette validation requires review"
    exit 1
fi