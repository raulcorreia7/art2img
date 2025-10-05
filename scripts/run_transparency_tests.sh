#!/bin/bash
# Comprehensive test runner for transparency functionality

set -e

echo "=== Comprehensive Transparency Test Runner ==="

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    local color=$1
    local message=$2
    echo -e "${color}${message}${NC}"
}

# Function to print section header
print_header() {
    local message=$1
    echo -e "${BLUE}=== ${message} ===${NC}"
}

# Check if build directory exists
if [ ! -d "build" ]; then
    print_status $YELLOW "Build directory not found, creating..."
    mkdir -p build
fi

# Build the project
print_header "Building Project"
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
cd ..

# Run unit tests specifically for transparency
print_header "Running Transparency Unit Tests"
cd build
ctest -R transparency -V
if [ $? -eq 0 ]; then
    print_status $GREEN "Unit tests passed!"
else
    print_status $RED "Unit tests failed!"
    exit 1
fi
cd ..

# Run the transparency validation script
print_header "Running Transparency Validation Script"
./scripts/test_transparency.sh

# Test with real assets if available
if [ -f "tests/assets/TILES000.ART" ] && [ -f "tests/assets/PALETTE.DAT" ]; then
    print_header "Testing with Real Assets"
    
    # Create test output directory
    OUTPUT_DIR="test_output/real_assets"
    mkdir -p "$OUTPUT_DIR"
    
    # Test with transparency enabled
    print_status $YELLOW "Testing with transparency enabled..."
    ./build/bin/art2img -f png -o "$OUTPUT_DIR/with_transparency" tests/assets/TILES000.ART
    
    # Test with transparency disabled
    print_status $YELLOW "Testing with transparency disabled..."
    ./build/bin/art2img -f png -o "$OUTPUT_DIR/no_transparency" -N tests/assets/TILES000.ART
    
    # Test with different formats
    print_status $YELLOW "Testing with BMP format..."
    ./build/bin/art2img -f bmp -o "$OUTPUT_DIR/bmp" tests/assets/TILES000.ART
    
    print_status $YELLOW "Testing with TGA format..."
    ./build/bin/art2img -f tga -o "$OUTPUT_DIR/tga" tests/assets/TILES000.ART
    
    # Report results
    print_header "Real Assets Test Results"
    echo "PNG with transparency: $(find "$OUTPUT_DIR/with_transparency" -name "*.png" | wc -l) files"
    echo "PNG without transparency: $(find "$OUTPUT_DIR/no_transparency" -name "*.png" | wc -l) files"
    echo "BMP files: $(find "$OUTPUT_DIR/bmp" -name "*.bmp" | wc -l) files"
    echo "TGA files: $(find "$OUTPUT_DIR/tga" -name "*.tga" | wc -l) files"
    
    print_status $GREEN "Real assets tests completed successfully!"
else
    print_status $YELLOW "Test assets not found, skipping real assets tests"
fi

# Test edge cases
print_header "Testing Edge Cases"
EDGE_CASE_DIR="test_output/edge_cases"
mkdir -p "$EDGE_CASE_DIR"

# Test with various tile sizes
print_status $YELLOW "Testing various output formats..."
./build/bin/art2img -f png -o "$EDGE_CASE_DIR/png" tests/assets/TILES000.ART
./build/bin/art2img -f bmp -o "$EDGE_CASE_DIR/bmp" tests/assets/TILES000.ART
./build/bin/art2img -f tga -o "$EDGE_CASE_DIR/tga" tests/assets/TILES000.ART

# Test with custom options
print_status $YELLOW "Testing with custom options..."
./build/bin/art2img -f png --no-fix-transparency -o "$EDGE_CASE_DIR/no_fix" tests/assets/TILES000.ART

print_header "Edge Cases Test Results"
echo "PNG files: $(find "$EDGE_CASE_DIR/png" -name "*.png" | wc -l)"
echo "BMP files: $(find "$EDGE_CASE_DIR/bmp" -name "*.bmp" | wc -l)"
echo "TGA files: $(find "$EDGE_CASE_DIR/tga" -name "*.tga" | wc -l)"
echo "No-fix PNG files: $(find "$EDGE_CASE_DIR/no_fix" -name "*.png" | wc -l)"

# Summary
print_header "Test Summary"
print_status $GREEN "All transparency tests completed successfully!"
print_status $YELLOW "Output files are available in test_output/ directory for manual inspection"

echo ""
echo "Next steps:"
echo "1. Manually inspect the generated files in test_output/ directory"
echo "2. Verify that magenta pixels are transparent in files with transparency enabled"
echo "3. Verify that magenta pixels are preserved in files with transparency disabled"
echo "4. Check that different formats handle transparency appropriately"

exit 0