#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Default values
clean_build=false
build=false
ARGS=()

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -clean|--clean-build) clean_build=true ;;
        -build|--build) build=true ;;
	*) NYTRO_ARGS+=("$1") ;;
    esac
    shift
done

# Clean build if requested
if [ "$clean_build" = true ]; then
    echo "--- Performing a clean build of Nytrogen ---"
    "$SCRIPT_DIR/run_scripts/clean_build.sh"
    echo "--- Clean build complete ---"
fi

# Incremental build if requested
if [ "$build" = true ]; then
    echo "--- Performing an incremental build of Nytrogen ---"
    "$SCRIPT_DIR/run_scripts/build.sh"
    echo "--- Incremental build complete ---"
fi

# Run the compiler
if [ ${#NYTRO_ARGS[@]} -gt 0 ]; then
    "$SCRIPT_DIR/build/bin/nytro" "${NYTRO_ARGS[@]}"
else
    # Only show usage if the user didn't just ask for a build
    if [ "$build" = false ] && [ "$clean_build" = false ]; then
        echo "Usage: $0 [-build] [-clean] <input_file.nyt> [flags]"
    fi
fi
