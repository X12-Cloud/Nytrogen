# Nytrogen TODO

## 0.132

### TODO
- **Implement Codegen for Switch Statements:** Currently only lexical, syntactic and semantic analysis is implemented.
- **Optimise the Compiler Core and Remake the Driver:** Make it more modular and abstract.
- **New Types:** Long as 64 bit integers, signed/unsigned qualifiers.
- **Fix Recursive Functions:** Functions should be able to be called inside themselves.
- **Low Level:** Implement a lot of low level stuff ig.
- **String concatenation** Implement string concatenation and substr with `+` and `-`.

### Done
- **Make the Backend more Abstract** Make a new InsructionSet/InstructionEmitter class for the CG to use.
- **Make a Register Allocator:** Continue optimising the compiler to always use registers when it can instead of using the stack for most stuff.
- **Fix and Stablise Casting:** Stablise type casting to not truncate stuff and be more sturdy.
- **Qlib:** Work on qlib.
