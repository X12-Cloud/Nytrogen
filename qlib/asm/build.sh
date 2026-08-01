#!/bin/bash
set -e

echo "--- Building QLib Locally ---"
mkdir -p out

# Compile
g++ -c qlib.cpp -o out/qlib.o -fPIC

# Assemble
nasm -f elf64 init.asm -o out/init.o
nasm -f elf64 g.asm -o out/g.o
nasm -f elf64 setup.asm -o out/setup.o
nasm -f elf64 measure.asm -o out/measure.o

# Create Archive
ar rcs out/libqlib.a out/init.o out/g.o out/setup.o out/measure.o out/qlib.o

echo "--- Success! Library created at ./out/libqlib.a ---"
