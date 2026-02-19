# Compiler Issues

This document outlines the current problems with the Nytrogen compiler.

## 1. Segmentation Fault

- **Symptom:** The compiled program crashes with a segmentation fault (exit code 139).
- **Source:** Incorrect memory access in the generated assembly code, originating from `src/code_generator.cpp`.

## 2. Incorrect Stack Management

- **Symptom:** The assembly code does not allocate space for local variables on the stack (e.g., `sub rsp, 0`).
- **Source:** The `visit(FunctionDefinitionNode*)` function in `src/code_generator.cpp` does not correctly calculate the size of local variables.

## 3. Incorrect Variable and Pointer Addressing

- **Symptom:** The assembly code uses incorrect positive offsets from the base pointer (`rbp`) to access local variables and pointers (e.g., `[rbp + 0]` instead of `[rbp - 8]`).
- **Source:** The `visit(VariableDeclarationNode*)`, `visit(VariableReferenceNode*)`, `visit(UnaryOpExpressionNode*)`, and `visit(ArrayAccessNode*)` functions in `src/code_generator.cpp` generate incorrect addresses.

## 4. Incorrect Array Handling

- **Symptom:** Accessing array elements fails due to incorrect address calculation in the assembly code.
- **Source:** The `visit(ArrayAccessNode*)` function in `src/code_generator.cpp` generates incorrect code for array access and improperly dereferences array element pointers.

## 5. Test Suite Failures

- **Symptom:** The `run_tests.sh` script fails for all tests with the error "ASM FILE NOT GENERATED".
- **Source:** The compiler writes all assembly output to a hardcoded file, `out/out.asm`, while the test script expects a unique assembly file for each test. This is caused by a hardcoded output filename in `src/main.cpp`.

# Solutions

Here is the plan to fix the issues:

## Step 1: Fix Local Variable Handling

1.  **Correct Offset Calculation:** Modify the `visit(FunctionDefinitionNode*)` function in `src/code_generator.cpp` to correctly calculate the total size of all local variables. Then, allocate the required space on the stack using `sub rsp, <size>`.
2.  **Use Negative Offsets:** Update the code to use negative offsets from the base pointer (`rbp`) for all local variable access. This will involve changes in:
    *   `visit(VariableDeclarationNode*)`
    *   `visit(VariableReferenceNode*)`
    *   `visit(UnaryOpExpressionNode*)`
    *   `visit(ArrayAccessNode*)`

## Step 2: Fix Array and Pointer Handling

1.  **Correct Array Element Addressing:** Fix the `visit(ArrayAccessNode*)` function to correctly calculate the address of an array element. This will involve using the correct base address of the array and the element size.
2.  **Remove Incorrect Dereferencing:** Remove the extra dereferencing in the `visit(ArrayAccessNode*)` function to ensure it returns the address of the element, not its value.

## Step 3: Fix the Test Suite

1.  **Parameterize Output Filename:** Modify `src/main.cpp` to accept the output assembly filename as a command-line argument. This will allow us to generate a separate assembly file for each test.
2.  **Update Test Script:** Update the `run_tests.sh` script to pass a unique output filename to the compiler for each test case. This will allow the test suite to correctly check the output of each test.
