#!/bin/bash
# Run tests for gAgent

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"
BUILD_DIR="$PROJECT_ROOT/build"

if [ ! -d "$BUILD_DIR" ]; then
    echo "Build directory not found. Run build.sh first."
    exit 1
fi

cd "$BUILD_DIR"

echo "Running tests..."
ctest --output-on-failure

echo "Tests completed!"
