#!/bin/bash
set -e

echo "--- Performing a clean build and then running test.ny ---"

# Step 1: Perform a clean build using clean-build.sh (relative to current script location)
./clean-build.sh

# Step 2: Compile the Nytrogen source file using the freshly built compiler
echo "--- Compiling test.ny ---"
./../build/Nytro ../test.ny

# Step 3: Assemble the generated assembly code
echo "--- Assembling out.asm ---"
nasm -f elf64 -o ../out.o ../out.asm

# Step 4: Link the assembled object file into an executable
echo "--- Linking out.o ---"
ld -e _start ../out.o -o ../out

# Step 5: Run the compiled program and display its exit code
echo "--- Running compiled program ---"
./../out
echo "Exit Code: $?"

echo "--- Clean run complete ---"
