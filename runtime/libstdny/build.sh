#!/bin/bash
set -e

# Path to the script
BASE_DIR=$(pwd)
OUT_DIR="$BASE_DIR/out"

echo "--- Building Nytrogen Standard Library (libstdny) ---"
rm -rf "$OUT_DIR"
mkdir -p "$OUT_DIR"

# Build Components
echo "Building Core..."
cd "$BASE_DIR/core" && ./build.sh

echo "Building QLib..."
cd "$BASE_DIR/qlib" && ./build.sh

# Aggregate all .o files into the master out/
echo "Aggregating object files..."
cp "$BASE_DIR/core/out/"*.o "$OUT_DIR/"
cp "$BASE_DIR/qlib/out/"*.o "$OUT_DIR/"

# Create the Unified Static Archive
echo "Creating archive libstdny.a..."
cd "$OUT_DIR"
ar rcs libstdny.a *.o
cp libstdny.a $BASE_DIR/libstdny.a

# Handle Installation
if [[ "$1" == "-install" ]]; then
    echo "Installing to /usr/local/lib..."
    sudo install -m 644 libstdny.a /usr/local/lib/libstdny.a
    echo "Done. You can now use -lstdny in your compiler driver."
else
    echo "Build complete. Library located at $OUT_DIR/libstdny.a"
fi
