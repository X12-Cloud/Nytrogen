#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Default values
file_viewer="bat"
clean_build=false
build=false
test=false
enable_fviewer=false
ARGS=()

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -cclean|--compiler-cleanbuild) clean_build=true ;;
        -cbuild|--compiler-build) build=true ;;
        -test|--test) test=true ;;
        -vf|--view-outputfile) enable_fviewer=true ;;
	*) NYTRO_ARGS+=("$1") ;;
    esac
    shift
done

# Clean build if requested
if [ "$clean_build" = true ]; then
    cd $SCRIPT_DIR/runtime/libstdny/ && ./build.sh -install && cd ../..
    "$SCRIPT_DIR/run_scripts/clean_build.sh"
fi

# Incremental build if requested
if [ "$build" = true ]; then
    cd $SCRIPT_DIR/runtime/libstdny/ && ./build.sh -install && cd ../..
    "$SCRIPT_DIR/run_scripts/build.sh"
fi

if [ "$test" = true ]; then
    "/$SCRIPT_DIR/tests/test.fish"
    EXIT_CODE=$?
    exit $EXIT_CODE
fi

# Run the compiler
"$SCRIPT_DIR/build/bin/nytro" "${NYTRO_ARGS[@]}"
EXIT_CODE=$?

# Viewer if requested
if [ "$enable_fviewer" = true ]; then
    echo "--- Printing output assembly file ---"
    $file_viewer "$SCRIPT_DIR/out/"*.asm
fi

exit $EXIT_CODE
