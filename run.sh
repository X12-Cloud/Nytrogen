#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Default values
file_viewer="bat"
clean_build=false
build=false
enable_fviewer=false
ARGS=()

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -cclean|--compiler-cleanbuild) clean_build=true ;;
        -cbuild|--compiler-build) build=true ;;
        -vf|--view-outputfile) enable_fviewer=true ;;
	*) NYTRO_ARGS+=("$1") ;;
    esac
    shift
done

# Clean build if requested
if [ "$clean_build" = true ]; then
    # echo "--- Performing a clean build of Nytrogen ---"
    "$SCRIPT_DIR/run_scripts/clean_build.sh"
    # echo "--- Clean build complete ---"
fi

# Incremental build if requested
if [ "$build" = true ]; then
    # echo "--- Performing an incremental build of Nytrogen ---"
    "$SCRIPT_DIR/run_scripts/build.sh"
    # echo "--- Incremental build complete ---"
fi

# Run the compiler
# if [ ${#NYTRO_ARGS[@]} -gt 0 ] || [ "$build" = true ] || [ "$clean_build" = true ]; then
"$SCRIPT_DIR/build/bin/nytro-driver" "${NYTRO_ARGS[@]}"
# else
    # echo "Usage: $0 [-cbuild/cclean] [-vf] <input_file.nyt>"
# fi

# Viewer if requested
if [ "$enable_fviewer" = true ]; then
    echo "--- Printing output assembly file ---"
    $file_viewer "$SCRIPT_DIR/out/"*.asm
fi

