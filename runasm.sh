#!/bin/bash

CWD=$(pwd)
OUT_DIR="$CWD/out"
mkdir -p "$OUT_DIR"

echo ""
echo "--- Assembling out.asm ---"
nasm -f elf64 "$1.asm" -o "$OUT_DIR/out.o"

echo ""
echo "--- Linking ---"
ld -o "$OUT_DIR/out" "$OUT_DIR/out.o" -lc --dynamic-linker /usr/lib64/ld-linux-x86-64.so.2

echo ""
echo "--- Running output program ---"
"$OUT_DIR/out"
echo "Exit Code: $?"
