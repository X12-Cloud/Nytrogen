#!/bin/bash

# Step 1: Perform an incremental build
cmake --build build

# Step 2: Run the Nytrogen source file
./build/Nytro test.nyt

# Steps 3 & 4: Assemble the generated asm file to an object file then link teh object file to an executable
nasm -f elf64 -o out.o out.asm
ld -e _start out.o -o out

# Step 5: Run the executable then print the output with echo
./out
echo  "Exit Code: $?"
