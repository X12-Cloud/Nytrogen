#!/bin/bash
cd "$(dirname "$0")"
set -e

echo "--- Performing a clean build of Nytrogen ---"

echo "Removing existing 'build' directory..."
rm -rf build

echo "Configuring and building Nytrogen compiler..."
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -- -j$(nproc)

echo "--- Clean build complete ---"

