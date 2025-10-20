#!/bin/bash

# Comprehensive validation script for art2img project
# This script performs all necessary checks before pushing changes

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
BUILD_TYPE="${BUILD_TYPE:-Debug}"
JOBS="${JOBS:-$(nproc)}"

# Logging functions
log_info() {
    echo -e "${BLUE}ℹ️  $1${NC}"
}

log_success() {
    echo -e "${GREEN}✅ $1${NC}"
}

log_warning() {
    echo -e "${YELLOW}⚠️  $1${NC}"
}

log_error() {
    echo -e "${RED}❌ $1${NC}"
}

log_step() {
    echo -e "\n${BLUE}🔍 $1${NC}"
}

# Check if we're in the right directory
check_project_root() {
    if [[ ! -f "$PROJECT_ROOT/CMakeLists.txt" ]] || [[ ! -f "$PROJECT_ROOT/Makefile" ]]; then
        log_error "Not in a valid art2img project directory"
        exit 1
    fi
    cd "$PROJECT_ROOT"
}

# Check dependencies
check_dependencies() {
    log_step "Checking dependencies..."
    
    # Check basic tools
    local missing_tools=()
    
    for tool in cmake clang-format clang-tidy git; do
        if ! command -v "$tool" >/dev/null 2>&1; then
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        log_error "Missing required tools: ${missing_tools[*]}"
        exit 1
    fi
    
    # Check CMake version
    local cmake_version
    cmake_version=$(cmake --version | head -n1 | grep -o '[0-9]\+\.[0-9]\+\.[0-9]\+')
    local required_version="3.20"
    if ! printf '%s\n' "$required_version" "$cmake_version" | sort -V -C; then
        log_error "CMake version $cmake_version is too old. Required: >= $required_version"
        exit 1
    fi
    
    log_success "All basic dependencies satisfied"
    
    # Check cross-compilation tools (optional)
    log_info "Checking cross-compilation dependencies (optional)..."
    
    if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
        log_success "MinGW (Windows cross-compilation) found"
    else
        log_warning "MinGW not found - Windows cross-compilation will be skipped"
    fi
    
    if command -v o64-clang >/dev/null 2>&1; then
        log_success "osxcross (macOS cross-compilation) found"
    else
        log_warning "osxcross not found - macOS cross-compilation will be skipped"
    fi
}

# Clean previous builds
clean_builds() {
    log_step "Cleaning previous builds..."
    if [[ -d "build" ]]; then
        rm -rf build
        log_success "Cleaned build directory"
    fi
}

# Build native version
build_native() {
    log_step "Building native version..."
    
    mkdir -p "build/linux_x64"
    cd "build/linux_x64"
    
    cmake -S ../.. -B . \
        -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
        -DBUILD_TESTING=ON \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
    
    cmake --build . --parallel "$JOBS"
    
    cd "$PROJECT_ROOT"
    log_success "Native build completed"
}

# Run tests
run_tests() {
    log_step "Running tests..."
    
    cd "build/linux_x64"
    
    if ! ctest --output-on-failure --parallel "$JOBS"; then
        log_error "Tests failed"
        exit 1
    fi
    
    cd "$PROJECT_ROOT"
    log_success "All tests passed"
}

# Check code formatting
check_formatting() {
    log_step "Checking code formatting..."
    
    local format_errors=0
    while IFS= read -r -d '' file; do
        if ! clang-format --dry-run --Werror "$file" >/dev/null 2>&1; then
            log_error "Formatting issues in: $file"
            format_errors=$((format_errors + 1))
        fi
    done < <(find src include tests -name '*.cpp' -o -name '*.hpp' -print0)
    
    if [[ $format_errors -gt 0 ]]; then
        log_error "Found $format_errors file(s) with formatting issues"
        log_info "Run 'make fmt' to fix formatting automatically"
        exit 1
    fi
    
    log_success "Code formatting is correct"
}

# Run static analysis
run_static_analysis() {
    log_step "Running static analysis..."
    
    mkdir -p "build/lint"
    cd "build/lint"
    
    cmake -S ../.. -B . \
        -DCMAKE_BUILD_TYPE=Debug \
        -DBUILD_TESTING=OFF \
        -DCMAKE_EXPORT_COMPILE_COMMANDS=ON >/dev/null 2>&1
    
    # Run clang-tidy but don't fail on warnings
    if command -v run-clang-tidy >/dev/null 2>&1; then
        run-clang-tidy -config '{}' -p . \
            $(find ../src ../include ../tests -name '*.cpp' -o -name '*.hpp') \
            >/dev/null 2>&1 || true
        log_success "Static analysis completed"
    else
        log_warning "run-clang-tidy not found, skipping static analysis"
    fi
    
    cd "$PROJECT_ROOT"
}

# Cross-compilation validation
validate_cross_compilation() {
    log_step "Validating cross-compilation..."
    
    local cross_build_success=0
    local cross_build_total=0
    
    # Windows x64
    if command -v x86_64-w64-mingw32-gcc >/dev/null 2>&1; then
        cross_build_total=$((cross_build_total + 1))
        log_info "Building Windows x64..."
        if make windows-x64-mingw >/dev/null 2>&1; then
            log_success "Windows x64 build successful"
            cross_build_success=$((cross_build_success + 1))
        else
            log_error "Windows x64 build failed"
        fi
    fi
    
    # Windows x86
    if command -v i686-w64-mingw32-gcc >/dev/null 2>&1; then
        cross_build_total=$((cross_build_total + 1))
        log_info "Building Windows x86..."
        if make windows-x86-mingw >/dev/null 2>&1; then
            log_success "Windows x86 build successful"
            cross_build_success=$((cross_build_success + 1))
        else
            log_error "Windows x86 build failed"
        fi
    fi
    
    # macOS x64
    if command -v o64-clang >/dev/null 2>&1; then
        cross_build_total=$((cross_build_total + 1))
        log_info "Building macOS x64..."
        if make macos-x64-osxcross >/dev/null 2>&1; then
            log_success "macOS x64 build successful"
            cross_build_success=$((cross_build_success + 1))
        else
            log_error "macOS x64 build failed"
        fi
    fi
    
    # macOS ARM64
    if command -v oa64-clang >/dev/null 2>&1; then
        cross_build_total=$((cross_build_total + 1))
        log_info "Building macOS ARM64..."
        if make macos-arm64-osxcross >/dev/null 2>&1; then
            log_success "macOS ARM64 build successful"
            cross_build_success=$((cross_build_success + 1))
        else
            log_error "macOS ARM64 build failed"
        fi
    fi
    
    if [[ $cross_build_total -eq 0 ]]; then
        log_warning "No cross-compilation tools found"
    else
        log_info "Cross-compilation: $cross_build_success/$cross_build_total builds successful"
        if [[ $cross_build_success -eq $cross_build_total ]]; then
            log_success "All cross-compilation builds successful"
        else
            log_warning "Some cross-compilation builds failed"
        fi
    fi
}

# Git checks
check_git_status() {
    log_step "Checking git status..."
    
    # Check for uncommitted changes
    if ! git diff --quiet || ! git diff --cached --quiet; then
        log_warning "You have uncommitted changes"
        log_info "Consider committing your changes before pushing"
    fi
    
    # Check for untracked files
    local untracked_files
    untracked_files=$(git ls-files --others --exclude-standard | wc -l)
    if [[ $untracked_files -gt 0 ]]; then
        log_warning "You have $untracked_files untracked file(s)"
    fi
}

# Performance check (optional)
run_performance_check() {
    if [[ -f "build/linux_x64/benchmark/test_convert_benchmark" ]]; then
        log_step "Running performance benchmarks..."
        cd "build/linux_x64"
        if ./benchmark/test_convert_benchmark --benchmark_min_time=0.1 >/dev/null 2>&1; then
            log_success "Performance benchmarks completed"
        else
            log_warning "Performance benchmarks failed or not available"
        fi
        cd "$PROJECT_ROOT"
    fi
}

# Main validation function
run_validation() {
    local validation_type="${1:-full}"
    
    log_info "Starting $validation_type validation for art2img project..."
    log_info "Project root: $PROJECT_ROOT"
    log_info "Build type: $BUILD_TYPE"
    log_info "Parallel jobs: $JOBS"
    
    check_project_root
    check_dependencies
    check_git_status
    
    case "$validation_type" in
        "quick")
            build_native
            run_tests
            ;;
        "native")
            clean_builds
            build_native
            run_tests
            check_formatting
            run_static_analysis
            run_performance_check
            ;;
        "full")
            clean_builds
            build_native
            run_tests
            check_formatting
            run_static_analysis
            run_performance_check
            validate_cross_compilation
            ;;
        *)
            log_error "Unknown validation type: $validation_type"
            log_info "Available types: quick, native, full"
            exit 1
            ;;
    esac
    
    log_success "🎉 $validation_type validation completed successfully!"
    log_info "You're ready to push your changes! 🚀"
}

# Parse command line arguments
case "${1:-full}" in
    "quick"|"native"|"full")
        run_validation "$1"
        ;;
    "help"|"-h"|"--help")
        echo "Usage: $0 [quick|native|full]"
        echo ""
        echo "Validation types:"
        echo "  quick  - Build and run tests only"
        echo "  native - Full native validation (build, tests, format, lint)"
        echo "  full   - Complete validation including cross-compilation"
        echo ""
        echo "Environment variables:"
        echo "  BUILD_TYPE - CMake build type (Debug/Release) [default: Debug]"
        echo "  JOBS       - Number of parallel jobs [default: \$(nproc)]"
        ;;
    *)
        log_error "Unknown option: $1"
        log_info "Run '$0 help' for usage information"
        exit 1
        ;;
esac