#!/bin/bash
set -e

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

echo "--- Performing an incremental build of Nytrogen Preprocessor ---"

cmake --build "$SCRIPT_DIR/build"

echo "--- Incremental build complete ---"