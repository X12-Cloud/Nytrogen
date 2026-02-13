#!/bin/bash
set -e

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

cd "$SCRIPT_DIR/.."

echo "--- Performing an incremental build of Nytrogen ---"

cmake --build build -j$(nproc)

echo "--- Incremental build complete ---"
