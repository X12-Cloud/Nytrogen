#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Default values
clean_build=false
build=false
input_file=""

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -clean|--clean-build) clean_build=true ;;
        -build|--build) build=true ;;
        *) input_file="$1" ;;
    esac
    shift
done

# Clean build if requested
if [ "$clean_build" = true ]; then
    echo "--- Performing a clean build of Nytrogen and Preprocessor ---"
    "$SCRIPT_DIR/run_scripts/clean_build.sh"
    (cd "$SCRIPT_DIR/Preprocessor" && ./clean_build.sh)
    (cd "$SCRIPT_DIR/driver" && ./build.sh)
    echo "--- Clean build complete ---"
    exit 0
fi

# Incremental build if requested
if [ "$build" = true ]; then
    echo "--- Performing an incremental build of Nytrogen and Preprocessor ---"
    "$SCRIPT_DIR/run_scripts/build.sh"
    (cd "$SCRIPT_DIR/Preprocessor" && ./build.sh)
    (cd "$SCRIPT_DIR/driver" && ./build.sh)
    echo "--- Incremental build complete ---"
    exit 0
fi

# Check if input file is provided
if [ -z "$input_file" ]; then
    echo "Usage: $0 [-clean] [-build] <input_file.nyt>"
    exit 1
fi

# Run compiler
"$SCRIPT_DIR/driver/build/nytro" "$input_file"
