#!/bin/bash
set -e

echo "--- Performing a clean build of Nytrogen Preprocessor ---"

# Navigate to the Preprocessor directory
cd "$(dirname "$0")"

echo "Removing existing 'build' directory..."
rm -rf build

echo "Configuring and building Nytrogen preprocessor..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(nproc)

echo "--- Clean build complete ---"
