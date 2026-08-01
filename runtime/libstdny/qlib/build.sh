#!/bin/bash
set -e
mkdir -p out

# Build all C++ files in src/
for f in src/*.cpp; do
    if [ -e "$f" ]; then
        echo "  [CXX] $f"
        g++ -c "$f" -o "out/$(basename "$f").o" -fPIC
    fi
done

# Build all ASM files in asm/
for f in asm/*.asm; do
    if [ -e "$f" ]; then
        echo "  [AS] $f"
        nasm -f elf64 "$f" -o "out/$(basename "$f").o"
    fi
done
