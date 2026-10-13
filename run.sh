#!/usr/bin/env bash
set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$SCRIPT_DIR"

if [ ! -f "build/ApexExpense" ]; then
    echo "Executable build/ApexExpense not found. Building now..."
    ./build.sh
fi

export LD_LIBRARY_PATH="/home/mount/.local/qt5/usr/lib/x86_64-linux-gnu:${LD_LIBRARY_PATH}"
export QT_PLUGIN_PATH="/home/mount/.local/qt5/usr/lib/x86_64-linux-gnu/qt5/plugins:${QT_PLUGIN_PATH}"

echo "Launching ApexExpense..."
exec ./build/ApexExpense "$@"
