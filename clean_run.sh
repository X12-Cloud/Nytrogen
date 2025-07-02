#!/bin/bash
# set -e ==> this works in the othe  rscripts but not here, dont really know why

echo "--- Performing a clean build and then running test.ny ---"

# Step 1: Perform a clean build using clean_run.sh
./run_scripts/clean_build.sh

# Step 2: Compile the Nytrogen source file using the freshly built compiler
echo "--- Compiling test.nyt ---"
./build/Nytro test.nyt

# Step 3: Assemble the generated assembly code
echo "--- Assembling out.asm ---"
nasm -f elf64 -o out.o out.asm

# Step 4: Link the assembled object file into an executable
echo "--- Linking out.o ---"
ld -dynamic-linker /lib64/ld-linux-x86-64.so.2 out.o -lc -o out

# Step 5: Run the compiled program and display its exit code
echo "--- Running compiled program ---"
./out
echo "Exit Code: $?"

echo "--- Clean run complete ---"
