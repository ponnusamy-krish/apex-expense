#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

echo "=========================================="
echo " Building ApexExpense GUI Application... "
echo "=========================================="

mkdir -p build
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j$(nproc)

echo ""
echo " Build successful! Executable is located at: build/ApexExpense"
echo " Run './run.sh' to launch the application."
