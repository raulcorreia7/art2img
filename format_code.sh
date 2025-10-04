#!/bin/bash

# Script to format C++ code using clang-format with Google style

# Find the clang-format executable
if command -v clang-format &> /dev/null; then
    CLANG_FORMAT="clang-format"
elif command -v clang-format-12 &> /dev/null; then
    CLANG_FORMAT="clang-format-12"
elif command -v clang-format-11 &> /dev/null; then
    CLANG_FORMAT="clang-format-11"
elif command -v clang-format-10 &> /dev/null; then
    CLANG_FORMAT="clang-format-10"
elif command -v clang-format-9 &> /dev/null; then
    CLANG_FORMAT="clang-format-9"
else
    echo "Error: clang-format is not installed or not found in PATH"
    echo "Please install clang-format, e.g.:"
    echo "  Ubuntu/Debian: sudo apt install clang-format"
    echo "  macOS: brew install clang-format"
    echo "  CentOS/RHEL: sudo yum install clang-format"
    exit 1
fi

echo "Using $CLANG_FORMAT for formatting..."

# Find all C++ source and header files
SOURCE_FILES=$(find include/ src/ cli/ tests/ -name "*.cpp" -o -name "*.h" -o -name "*.hpp" -o -name "*.c" | grep -v "/build/" || true)

if [ -z "$SOURCE_FILES" ]; then
    echo "No source files found to format."
    exit 0
fi

echo "Formatting the following files:"
echo "$SOURCE_FILES"

# Format each file
for file in $SOURCE_FILES; do
    echo "Formatting $file..."
    "$CLANG_FORMAT" -i "$file"
done

echo "Code formatting complete!"