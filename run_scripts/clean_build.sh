#!/bin/bash
set -e

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Move to the root project directory
cd "$SCRIPT_DIR/.."

echo "--- Performing a clean build of Nytrogen ---"

echo "Removing existing 'build', 'out' directories..."
rm -rf out/*
rm -rf build

# Ensure out directory exists for the compiler tests later
mkdir -p out

echo "Configuring Nytrogen with Lua 5.4..."
# Combine all flags into one configuration call
cmake -B build \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  -DLUA_INCLUDE_DIR=/usr/include/lua5.4 \
  -DLUA_LIBRARY=/usr/lib/liblua5.4.so \
  .

echo "Building components..."
# Build everything inside the specified build directory
cmake --build build -j$(nproc)

echo "--- Clean build complete ---"
