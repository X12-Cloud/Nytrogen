#!/bin/bash
set -e

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

echo "--- Performing a clean build of Nytrogen Preprocessor ---"


echo "Removing existing 'build' directory..."
rm -rf "$SCRIPT_DIR/build"


echo "Configuring and building Nytrogen preprocessor..."
cmake -B "$SCRIPT_DIR/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$SCRIPT_DIR/build" -- -j$(nproc)

echo "--- Clean build complete ---"
