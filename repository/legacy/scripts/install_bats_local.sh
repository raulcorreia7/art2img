#!/bin/bash

# =============================================================================
# Install Bats-core locally to the project
# =============================================================================

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
LOCAL_BATS_DIR="$PROJECT_ROOT/.bin/bats"

# Create local binary directory
mkdir -p "$LOCAL_BATS_DIR"

echo "Installing Bats-core to $LOCAL_BATS_DIR..."

# Clone Bats-core to a temporary directory
TEMP_DIR=$(mktemp -d)
git clone https://github.com/bats-core/bats-core.git "$TEMP_DIR"

# Install Bats to local directory
"$TEMP_DIR/install.sh" "$LOCAL_BATS_DIR"

# Clean up
rm -rf "$TEMP_DIR"

echo "Bats-core installed successfully!"
echo "Bats executable: $LOCAL_BATS_DIR/bin/bats"
echo ""
echo "Add to your PATH: export PATH=\"$LOCAL_BATS_DIR:\$PATH\""