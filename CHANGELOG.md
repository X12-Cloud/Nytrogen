# Nytrogen Compiler

## 0.120 (Current)
- Add support for mulit-variable declarations `int: x, y, z;` optionally `int: x = 1, y = 2, z;`.
- Added the debug (`-debug`) flag so all those debug lines are optional.
- Added the S/asm flag (`-S` or `-asm`) to stop after generating the `.asm` file.
- Cleaned up some stuff in the driver and run script.

## 0.111
- Fix infinite loop in `stress_test` by correcting register mismatch in `BinaryOperationExpressionNode`.
- Fix variable corruption in assignments by saving RHS results before evaluating LHS addresses.
- Fix double support

## 0.110
- Added support for `float` (32 bit) and `double` (64 bit) keywords.
- Hardware Acceleration: Implemented XMM register usage and SSE instructions (`vaddss`, `vsubss`, etc.).
- Type-Aware Backend: New logic to peak inside smart pointers for AST nodes to determine register paths.
- Print logic: Refactored the `print` statement's code generation to be easier to scale.

## 0.100 (Ignition)
- First official release.
