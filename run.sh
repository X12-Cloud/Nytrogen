#!/bin/bash

if [ -z "$1" ]; then
    echo "Usage: $0 <input_file.nyt>"
    exit 1
fi

SCRIPT_DIR=$(dirname "$(readlink -f "$0")")

# Create output directory if it doesn't exist
mkdir -p out

# Resolve input file path to be absolute
INPUT_FILE=$(readlink -f "$1")

"$SCRIPT_DIR/build/Nytro" "$INPUT_FILE"

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