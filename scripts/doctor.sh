#!/bin/bash

# =============================================================================
# art2img Doctor Script
# =============================================================================
# This script verifies that all dependencies required to build art2img are installed
# Usage: ./scripts/doctor.sh
# =============================================================================

set -e

# Colors for output
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly NC='\033[0m' # No Color

# Logging functions
log_info() {
    echo -e "${BLUE}ℹ${NC} $1"
}

log_success() {
    echo -e "${GREEN}✓${NC} $1"
}

log_warning() {
    echo -e "${YELLOW}⚠${NC} $1"
}

log_error() {
    echo -e "${RED}✗${NC} $1"
}

# Check if a command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Check if a package is installed (Debian/Ubuntu)
package_exists_debian() {
    dpkg -l | grep -q "^ii  $1 "
}

# Check if a package is installed (RedHat/Fedora)
package_exists_redhat() {
    rpm -q "$1" >/dev/null 2>&1
}

# Check if a package is installed (Arch)
package_exists_arch() {
    pacman -Q "$1" >/dev/null 2>&1
}

# Detect OS distribution
detect_distribution() {
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        echo "$ID"
    elif command_exists lsb_release; then
        lsb_release -si
    else
        echo "unknown"
    fi
}

# Check build essentials
check_build_essentials() {
    log_info "Checking build essentials..."
    
    local missing_tools=()
    local required_tools=("gcc" "g++" "make" "cmake" "pkg-config")
    
    for tool in "${required_tools[@]}"; do
        if command_exists "$tool"; then
            log_success "Found $tool: $(eval "$tool --version" | head -n1)"
        else
            log_error "Missing $tool"
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        echo
        log_warning "Missing build essentials: ${missing_tools[*]}"
        return 1
    fi
    
    return 0
}

# Check cross-compilation tools
check_cross_compilation_tools() {
    log_info "Checking cross-compilation tools..."
    
    local missing_tools=()
    local required_tools=(
        "i686-w64-mingw32-gcc"
        "i686-w64-mingw32-g++"
        "x86_64-w64-mingw32-gcc"
        "x86_64-w64-mingw32-g++"
        "i686-w64-mingw32-windres"
        "x86_64-w64-mingw32-windres"
    )
    
    for tool in "${required_tools[@]}"; do
        if command_exists "$tool"; then
            log_success "Found $tool"
        else
            log_warning "Missing $tool (needed for Windows cross-compilation)"
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        echo
        log_info "To install cross-compilation tools on Ubuntu/Debian:"
        echo "  sudo apt-get install g++-mingw-w64-i686 g++-mingw-w64-x86-64"
        echo
        log_info "To install cross-compilation tools on Fedora:"
        echo "  sudo dnf install mingw32-gcc mingw32-gcc-c++ mingw64-gcc mingw64-gcc-c++"
        return 1
    fi
    
    return 0
}

# Check development libraries
check_development_libraries() {
    log_info "Checking development libraries..."
    
    # Check for basic development libraries
    if command_exists pkg-config; then
        log_success "Found pkg-config"
    else
        log_warning "Missing pkg-config"
    fi
    
    return 0
}

# Check compression tools
check_compression_tools() {
    log_info "Checking compression tools..."
    
    local missing_tools=()
    local required_tools=("zip" "tar" "gzip")
    
    for tool in "${required_tools[@]}"; do
        if command_exists "$tool"; then
            log_success "Found $tool"
        else
            log_warning "Missing $tool"
            missing_tools+=("$tool")
        fi
    done
    
    if [[ ${#missing_tools[@]} -gt 0 ]]; then
        echo
        log_info "To install compression tools on Ubuntu/Debian:"
        echo "  sudo apt-get install zip tar gzip"
        return 1
    fi
    
    return 0
}

# Check testing tools
check_testing_tools() {
    log_info "Checking testing tools..."
    
    # Wine for Windows testing
    if command_exists wine; then
        log_success "Found Wine ($(wine --version))"
    else
        log_warning "Missing Wine (needed for Windows binary testing)"
        echo "To install Wine on Ubuntu/Debian:"
        echo "  sudo apt-get install wine"
    fi
    
    return 0
}

# Check GitHub CLI for release management
check_github_cli() {
    log_info "Checking GitHub CLI..."
    
    if command_exists gh; then
        log_success "Found GitHub CLI ($(gh --version | head -n1))"
    else
        log_info "GitHub CLI not found (optional for release management)"
        echo "To install GitHub CLI on Ubuntu/Debian:"
        echo "  sudo apt-get install gh"
    fi
    
    return 0
}

# Check Python for potential scripts
check_python() {
    log_info "Checking Python..."
    
    if command_exists python3; then
        log_success "Found Python 3 ($(python3 --version))"
    else
        log_warning "Python 3 not found (optional for some scripts)"
        echo "To install Python 3 on Ubuntu/Debian:"
        echo "  sudo apt-get install python3"
    fi
    
    return 0
}

# Check shellcheck for script linting
check_shellcheck() {
    log_info "Checking shellcheck..."
    
    if command_exists shellcheck; then
        log_success "Found shellcheck ($(shellcheck --version | grep version | head -n1))"
    else
        log_info "shellcheck not found (optional for script linting)"
        echo "To install shellcheck on Ubuntu/Debian:"
        echo "  sudo apt-get install shellcheck"
    fi
    
    return 0
}

# Check if test assets are present
check_test_assets() {
    log_info "Checking test assets..."
    
    local test_assets_dir="tests/assets"
    local required_files=("PALETTE.DAT" "TILES000.ART")
    
    if [[ ! -d "$test_assets_dir" ]]; then
        log_error "Test assets directory not found: $test_assets_dir"
        return 1
    fi
    
    local missing_files=()
    for file in "${required_files[@]}"; do
        if [[ -f "$test_assets_dir/$file" ]]; then
            log_success "Found $file ($(stat -f%z "$test_assets_dir/$file" 2>/dev/null || stat -c%s "$test_assets_dir/$file") bytes)"
        else
            log_error "Missing $file"
            missing_files+=("$file")
        fi
    done
    
    if [[ ${#missing_files[@]} -gt 0 ]]; then
        log_error "Missing test assets: ${missing_files[*]}"
        echo "Please ensure test assets are present in $test_assets_dir"
        return 1
    fi
    
    return 0
}

# Main system check
check_system() {
    log_info "Checking system information..."
    
    # OS information
    if [[ -f /etc/os-release ]]; then
        . /etc/os-release
        log_success "Operating System: $PRETTY_NAME"
    else
        log_success "Operating System: $(uname -s) $(uname -r)"
    fi
    
    # Architecture
    log_success "Architecture: $(uname -m)"
    
    # CPU cores
    local cores
    if command_exists nproc; then
        cores=$(nproc)
    else
        cores=$(sysctl -n hw.ncpu 2>/dev/null || echo "unknown")
    fi
    log_success "CPU cores: $cores"
    
    return 0
}

# Installation suggestions based on OS
show_installation_suggestions() {
    local distro
    distro=$(detect_distribution)
    
    echo
    log_info "Installation suggestions for your system:"
    
    case "$distro" in
        ubuntu|debian)
            echo "For Ubuntu/Debian systems, run:"
            echo "  sudo apt-get update"
            echo "  sudo apt-get install \\"
            echo "    build-essential \\"
            echo "    cmake \\"
            echo "    pkg-config \\"
            echo "    g++-mingw-w64-i686 \\"
            echo "    g++-mingw-w64-x86-64 \\"
            echo "    zip \\"
            echo "    wine"
            ;;
        fedora)
            echo "For Fedora systems, run:"
            echo "  sudo dnf install \\"
            echo "    gcc gcc-c++ make cmake \\"
            echo "    pkgconfig \\"
            echo "    mingw32-gcc mingw32-gcc-c++ \\"
            echo "    mingw64-gcc mingw64-gcc-c++ \\"
            echo "    zip \\"
            echo "    wine"
            ;;
        arch)
            echo "For Arch Linux systems, run:"
            echo "  sudo pacman -S \\"
            echo "    base-devel \\"
            echo "    cmake \\"
            echo "    pkgconf \\"
            echo "    mingw-w64-gcc \\"
            echo "    zip \\"
            echo "    wine"
            ;;
        *)
            echo "Please install the equivalent packages for your distribution."
            echo "Required packages: gcc, g++, make, cmake, pkg-config, mingw-w64, zip"
            ;;
    esac
}

# Main function
main() {
    echo -e "${BLUE}art2img Doctor Script${NC}"
    echo "======================"
    echo "Checking system for art2img build dependencies..."
    echo
    
    # Track overall status
    local overall_status=0
    
    # Check system information
    check_system || overall_status=1
    echo
    
    # Check build essentials
    check_build_essentials || overall_status=1
    echo
    
    # Check cross-compilation tools
    check_cross_compilation_tools || overall_status=1
    echo
    
    # Check development libraries
    check_development_libraries || overall_status=1
    echo
    
    # Check compression tools
    check_compression_tools || overall_status=1
    echo
    
    # Check testing tools
    check_testing_tools || overall_status=1
    echo
    
    # Check optional tools
    check_github_cli || overall_status=1
    check_python || overall_status=1
    check_shellcheck || overall_status=1
    echo
    
    # Check test assets
    check_test_assets || overall_status=1
    echo
    
    # Summary
    if [[ $overall_status -eq 0 ]]; then
        log_success "All required dependencies are installed! You're ready to build art2img."
        echo
        log_info "Try running: make all"
    else
        log_warning "Some dependencies are missing or incomplete."
        show_installation_suggestions
        echo
        log_info "After installing the required dependencies, run this script again to verify."
    fi
    
    return $overall_status
}

# Run main function
main "$@"