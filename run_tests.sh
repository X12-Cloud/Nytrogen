#!/bin/bash

# Exit immediately if a command exits with a non-zero status.
set -e

# Define colors for output
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}--- Building Nytrogen Compiler ---${NC}"
./build.sh

COMPILER_PATH="./build/Nytro"
OUTPUT_DIR="./out"
TESTS_DIR="./tests"

mkdir -p "$OUTPUT_DIR"

PASSED_COUNT=0
FAILED_COUNT=0

echo -e "\n${YELLOW}--- Running Tests ---${NC}"

for test_file in "$TESTS_DIR"/*.nyt; do
    if [ -f "$test_file" ]; then
        test_name=$(basename "$test_file" .nyt)
        expected_output_file="$TESTS_DIR/${test_name}.expected"
        actual_output_file="$OUTPUT_DIR/${test_name}.actual"
        asm_output_file="$OUTPUT_DIR/${test_name}.asm"
        obj_output_file="$OUTPUT_DIR/${test_name}.o"
        exec_output_file="$OUTPUT_DIR/${test_name}.out"

        echo -n "Running test: ${test_name}... "

        # 1. Compile Nytrogen source to assembly
        if ! "$COMPILER_PATH" "$test_file"; then
            echo -e "${RED}COMPILATION FAILED${NC}"
            FAILED_COUNT=$((FAILED_COUNT + 1))
            continue
        fi

        # Ensure the assembly file was created
        if [ ! -f "$asm_output_file" ]; then
            echo -e "${RED}ASM FILE NOT GENERATED${NC}"
            FAILED_COUNT=$((FAILED_COUNT + 1))
            continue
        fi

        # 2. Assemble the assembly file
        if ! nasm -f elf64 "$asm_output_file" -o "$obj_output_file" > /dev/null 2>&1; then
            echo -e "${RED}ASSEMBLY FAILED${NC}"
            FAILED_COUNT=$((FAILED_COUNT + 1))
            continue
        fi

        # 3. Link the object file
        if ! ld -o "$exec_output_file" "$obj_output_file" > /dev/null 2>&1; then
            echo -e "${RED}LINKING FAILED${NC}"
            FAILED_COUNT=$((FAILED_COUNT + 1))
            continue
        fi

        # 4. Execute the compiled program and capture output
        if ! "$exec_output_file" > "$actual_output_file" 2>&1; then
            echo -e "${RED}EXECUTION FAILED${NC}"
            FAILED_COUNT=$((FAILED_COUNT + 1))
            continue
        fi

        # 5. Compare with expected output
        if [ ! -f "$expected_output_file" ]; then
            echo -e "${YELLOW}WARNING: Expected output file not found. Creating it.${NC}"
            cp "$actual_output_file" "$expected_output_file"
            echo -e "${GREEN}PASSED (Expected file created)${NC}"
            PASSED_COUNT=$((PASSED_COUNT + 1))
        elif diff -q "$expected_output_file" "$actual_output_file" > /dev/null; then
            echo -e "${GREEN}PASSED${NC}"
            PASSED_COUNT=$((PASSED_COUNT + 1))
        else
            echo -e "${RED}FAILED (Output Mismatch)${NC}"
            echo -e "${YELLOW}--- Diff for ${test_name} ---${NC}"
            diff "$expected_output_file" "$actual_output_file"
            echo -e "${YELLOW}--------------------------${NC}"
            FAILED_COUNT=$((FAILED_COUNT + 1))
        fi
    fi
done

echo -e "\n${YELLOW}--- Test Summary ---${NC}"
echo -e "${GREEN}Passed: ${PASSED_COUNT}${NC}"
echo -e "${RED}Failed: ${FAILED_COUNT}${NC}"

if [ "$FAILED_COUNT" -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed.${NC}"
    exit 1
fi
