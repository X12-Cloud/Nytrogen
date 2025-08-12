#!/bin/bash

./build/Nytro "test_structs.nyt"

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

