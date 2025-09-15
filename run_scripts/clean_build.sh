#!/bin/bash
set -e

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

echo "--- Performing a clean build of Nytrogen ---"


echo "Removing existing 'build' directory..."
rm -rf "$SCRIPT_DIR/../build"


echo "Configuring and building Nytrogen compiler..."
cmake -B "$SCRIPT_DIR/../build"
cmake --build "$SCRIPT_DIR/../build"

echo "--- Clean build complete ---"
