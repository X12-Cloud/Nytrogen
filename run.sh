#!/bin/bash

# Get the absolute path of the script's directory
SCRIPT_DIR=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" &> /dev/null && pwd)

# Default values
disable_preprocessor=false
clean_build=false
build=false
input_file=""

# Parse command-line arguments
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -dp|--disable-preprocessor) disable_preprocessor=true ;;
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
    echo "--- Clean build complete ---"
    exit 0
fi

# Incremental build if requested
if [ "$build" = true ]; then
    echo "--- Performing an incremental build of Nytrogen and Preprocessor ---"
    "$SCRIPT_DIR/run_scripts/build.sh"
    (cd "$SCRIPT_DIR/Preprocessor" && ./build.sh)
    echo "--- Incremental build complete ---"
    exit 0
fi

# Check if input file is provided
if [ -z "$input_file" ]; then
    echo "Usage: $0 [-dp] [-clean] [-build] <input_file.nyt>"
    exit 1
fi

# Create out directory in the current working directory
CWD=$(pwd)
OUT_DIR="$CWD/out"
mkdir -p "$OUT_DIR"

# Run preprocessor if not disabled
if [ "$disable_preprocessor" = false ]; then
    echo "--- Running Nytrogen Preprocessor ---"
    preprocessed_file="$OUT_DIR/$(basename "${input_file%.nyt}.pre.nyt")"
    "$SCRIPT_DIR/Preprocessor/run.sh" "$input_file" > "$preprocessed_file"
    echo "--- Preprocessor finished ---"
else
    preprocessed_file="$input_file"
fi

# Run compiler
echo "--- Running Nytrogen Compiler ---"
"$SCRIPT_DIR/bootstrap/build/Nytro" "$preprocessed_file" "$OUT_DIR/out.asm"

echo ""
echo "--- Assembling out.asm ---"
nasm -f elf64 "$OUT_DIR/out.asm" -o "$OUT_DIR/out.o"

echo ""
echo "--- Linking ---"
ld -o "$OUT_DIR/out" "$OUT_DIR/out.o" -lc --dynamic-linker /usr/lib64/ld-linux-x86-64.so.2

echo ""
echo "--- Running output program ---"
"$OUT_DIR/out"
echo "Exit Code: $?"