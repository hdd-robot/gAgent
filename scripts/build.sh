#!/bin/bash
# Build script for gAgent

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

BUILD_TYPE="${1:-Release}"
BUILD_DIR="$PROJECT_ROOT/build"

echo "Building gAgent in $BUILD_TYPE mode..."

# Create build directory
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
    -DBUILD_TESTS=ON \
    -DBUILD_EXAMPLES=ON \
    -ENABLE_WARNINGS=ON

# Build
make -j$(nproc)

echo "Build completed successfully!"
echo "Binaries are in $BUILD_DIR"
