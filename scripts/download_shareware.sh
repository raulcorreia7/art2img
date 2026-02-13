#!/bin/bash
# Duke Nukem 3D Shareware Downloader
# Downloads and extracts DUKE3D.GRP from Archive.org
#
# Usage:
#   ./download_shareware.sh              # Download to default location
#   ./download_shareware.sh /path/to/dir # Download to specific directory

set -euo pipefail

# ============================================================================
# Configuration
# ============================================================================

readonly SHAREWARE_URL="https://archive.org/download/3D_Realms_Duke_Nukem_3D_Shareware/3D%20Realms%20-%20Duke%20Nukem%203D%20%28Shareware%20Version%29.zip"
readonly EXPECTED_GRP_SIZE=$((10 * 1024 * 1024))  # ~10MB minimum

# ============================================================================
# Logging
# ============================================================================

log_info()  { echo "[INFO]  $*" >&2; }
log_error() { echo "[ERROR] $*" >&2; }
log_ok()    { echo "[OK]    $*" >&2; }

# ============================================================================
# Utility Functions
# ============================================================================

# Get the project root directory (parent of scripts/)
get_project_root() {
    local script_dir
    script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
    dirname "$script_dir"
}

# Check if required tools are available
check_dependencies() {
    local missing=()
    
    if ! command -v curl &>/dev/null && ! command -v wget &>/dev/null; then
        missing+=("curl or wget")
    fi
    
    if ! command -v unzip &>/dev/null; then
        missing+=("unzip")
    fi
    
    if [[ ${#missing[@]} -gt 0 ]]; then
        log_error "Missing required tools: ${missing[*]}"
        exit 1
    fi
}

# Download a file using curl or wget
download_file() {
    local url="$1"
    local output="$2"
    
    if command -v curl &>/dev/null; then
        curl -fsSL -o "$output" "$url"
    else
        wget -q -O "$output" "$url"
    fi
}

# Verify GRP file signature is valid
verify_grp() {
    local grp_file="$1"
    
    if [[ ! -f "$grp_file" ]]; then
        return 1
    fi
    
    local size
    size=$(stat -f%z "$grp_file" 2>/dev/null || stat -c%s "$grp_file" 2>/dev/null)
    
    if [[ $size -lt $EXPECTED_GRP_SIZE ]]; then
        log_error "GRP file too small: $size bytes (expected ~10MB)"
        return 1
    fi
    
    # Check signature
    local sig
    sig=$(head -c 12 "$grp_file" 2>/dev/null)
    if [[ "$sig" != "KenSilverman" ]]; then
        log_error "Invalid GRP signature"
        return 1
    fi
    
    return 0
}

# ============================================================================
# Main Operations
# ============================================================================

# Download the shareware zip file
download_shareware() {
    local output_dir="$1"
    local zip_file="$2"
    
    log_info "Downloading Duke3D Shareware from Archive.org..."
    
    if ! download_file "$SHAREWARE_URL" "$zip_file"; then
        log_error "Download failed"
        return 1
    fi
    
    log_ok "Downloaded: $(basename "$zip_file") ($(du -h "$zip_file" | cut -f1))"
    return 0
}

# Extract DUKE3D.GRP from the zip
extract_grp() {
    local zip_file="$1"
    local output_dir="$2"
    
    log_info "Extracting DUKE3D.GRP..."
    
    # Find GRP in zip and extract
    if ! unzip -j "$zip_file" "*/DUKE3D.GRP" -d "$output_dir" 2>/dev/null; then
        # Try without subdirectory
        unzip -j "$zip_file" "DUKE3D.GRP" -d "$output_dir" 2>/dev/null || {
            log_error "Could not find DUKE3D.GRP in archive"
            return 1
        }
    fi
    
    return 0
}

# Main entry point
main() {
    local output_dir
    
    # Determine output directory
    if [[ $# -gt 0 ]]; then
        output_dir="$1"
    else
        output_dir="$(get_project_root)/tests/shareware"
    fi
    
    local zip_file="$output_dir/duke3d_shareware.zip"
    local grp_file="$output_dir/DUKE3D.GRP"
    
    log_info "Output directory: $output_dir"
    
    # Check if already exists
    if [[ -f "$grp_file" ]] && verify_grp "$grp_file"; then
        log_ok "DUKE3D.GRP already exists"
        ls -lh "$grp_file"
        return 0
    fi
    
    # Setup
    check_dependencies
    if ! mkdir -p "$output_dir"; then
        log_error "Failed to create output directory: $output_dir"
        exit 1
    fi
    
    # Download
    if ! download_shareware "$output_dir" "$zip_file"; then
        exit 1
    fi
    
    # Extract
    if ! extract_grp "$zip_file" "$output_dir"; then
        rm -f "$zip_file"
        exit 1
    fi
    
    # Cleanup
    rm -f "$zip_file"
    
    # Verify
    if ! verify_grp "$grp_file"; then
        log_error "Extracted file verification failed"
        exit 1
    fi
    
    log_ok "Successfully extracted: $grp_file"
    ls -lh "$grp_file"
    
    return 0
}

# ============================================================================
# Entry Point
# ============================================================================

# Run if executed directly (not sourced)
if [[ "${BASH_SOURCE[0]}" == "${0}" ]]; then
    main "$@"
fi
