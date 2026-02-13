#!/bin/bash
set -e

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

cd "$SCRIPT_DIR/.."

echo "--- Performing a clean build of Nytrogen ---"


echo "Removing existing 'build', 'out' directories..."
rm -rf out/*
rm -rf build


echo "Configuring and building Nytrogen compiler..."
cmake -B build
cmake --build build -j$(nproc)

echo "--- Clean build complete ---"
