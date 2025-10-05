#!/bin/bash
# Test script for art2img conversion

# Create output directory
OUTPUT_DIR="test_output"
mkdir -p "$OUTPUT_DIR"

# Run conversion
echo "Running art2img conversion test..."
./build/linux-release/bin/art2img -f png -o "$OUTPUT_DIR" tests/assets/TILES000.ART

# Check results
if [ -d "$OUTPUT_DIR" ] && [ "$(ls -A $OUTPUT_DIR)" ]; then
    echo "Test successful! Output files created in $OUTPUT_DIR"
    echo "File count: $(ls -1 $OUTPUT_DIR | wc -l)"
else
    echo "Test failed! No output files found"
    exit 1
fi