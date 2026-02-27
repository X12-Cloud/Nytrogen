# Nytrogen Compiler

## 0.111 (Upcoming Patch)
- Fix infinite loop in `while` loops by correcting register mismatch in `BinaryOperationExpressionNode`.
- Fix variable corruption in assignments by saving RHS results before evaluating LHS addresses.
- Add support for 64-bit `double` types in the codegen.

## 0.110 (Current)
- Added support for `float` (32 bit) and `double` (64 bit) keywords.
- Hardware Acceleration: Implemented XMM register usage and SSE instructions (`vaddss`, `vsubss`, etc.).
- Type-Aware Backend: New logic to peak inside smart pointers for AST nodes to determine register paths.
- Print logic: Refactored the `print` statement's code generation to be easier to scale.

## 0.100 (Ignition)
- First official release.
