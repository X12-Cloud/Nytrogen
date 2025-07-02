#!/bin/bash

echo ""
./build/Nytro test.nyt

echo ""
echo "🔧 Assembling out.asm..."
nasm -f elf64 -o out.o out.asm

echo "🔗 Linking..."
ld -o out out.o -lc -I/lib/x86_64-linux-gnu --dynamic-linker /lib64/ld-linux-x86-64.so.2 -e _start

echo ""
echo "🚀 Running output program:"
./out
echo "Exit Code: $?"

