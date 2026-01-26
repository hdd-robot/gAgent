#!/bin/bash
# Format all C++ code using clang-format

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

echo "Formatting C++ code in $PROJECT_ROOT..."

find "$PROJECT_ROOT/src_agent" \
     "$PROJECT_ROOT/src_manager" \
     "$PROJECT_ROOT/test_agent" \
     "$PROJECT_ROOT/examples" \
     -type f \( -name "*.cpp" -o -name "*.hpp" -o -name "*.h" \) \
     -exec clang-format -i {} +

echo "Code formatting completed!"
