#!/bin/fish

rm -rf out/
mkdir out/

# 1. Assemble the qlib files
nasm -f elf64 init.asm -o out/init.o
nasm -f elf64 g.asm -o out/g.o
nasm -f elf64 setup.asm -o out/setup.o

# 2. Compile the C++ harness
g++ -c test_qlib.cpp -o out/test_qlib.o

# 3. Link them all together
g++ out/init.o out/g.o out/setup.o out/test_qlib.o -o out/qtest -mavx
