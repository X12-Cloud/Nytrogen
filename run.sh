#!/bin/bash

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
    ./run_scripts/clean_build.sh
    (cd Preprocessor && ./build.sh)
    echo "--- Clean build complete ---"
    exit 0
fi

# Incremental build if requested
if [ "$build" = true ]; then
    echo "--- Performing an incremental build of Nytrogen and Preprocessor ---"
    ./run_scripts/build.sh
    (cd Preprocessor && ./build.sh)
    echo "--- Incremental build complete ---"
    exit 0
fi

# Check if input file is provided
if [ -z "$input_file" ]; then
    echo "Usage: $0 [-dp] [-clean] [-build] <input_file.nyt>"
    exit 1
fi

# Create out directory if it doesn't exist
mkdir -p out

# Run preprocessor if not disabled
if [ "$disable_preprocessor" = false ]; then
    echo "--- Running Nytrogen Preprocessor ---"
    preprocessed_file="out/$(basename "${input_file%.nyt}.pre.nyt")"
    ./Preprocessor/run.sh "$input_file" > "$preprocessed_file"
    echo "--- Preprocessor finished ---"
else
    preprocessed_file="$input_file"
fi

# Run compiler
echo "--- Running Nytrogen Compiler ---"
./build/Nytro "$preprocessed_file"

echo ""
echo "--- Assembling out.asm ---"
nasm -f elf64 out/out.asm -o out/out.o

echo ""
echo "--- Linking ---"
ld -o out/out out/out.o -lc --dynamic-linker /usr/lib64/ld-linux-x86-64.so.2

echo ""
echo "--- Running output program ---"
./out/out
echo "Exit Code: $?"