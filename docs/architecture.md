# Nytrogen Compiler Architecture

This document provides a high-level overview of the Nytrogen compiler's internal architecture. Understanding this architecture is helpful for contributing to the project or for learning about compiler design.

The Nytrogen compiler follows a traditional multi-stage pipeline, where the source code is transformed through a series of phases until executable code is generated. Each phase has a distinct responsibility:

```
[Nytrogen Source Code] -> [Lexer] -> [Parser] -> [Semantic Analyzer] -> [Code Generator] -> [Executable Code]
```

## 1. Lexical Analysis (Lexer)

*   **Component:** `Lexer`
*   **Source Files:** `src/lexer.cpp`, `include/lexer.hpp`

**Responsibility:** The lexer is the first phase of the compiler. It reads the raw Nytrogen source code as a stream of characters and converts it into a sequence of tokens. Each token represents a single atomic unit of the language, such as a keyword (`if`, `while`), an identifier (`my_variable`), a number (`123`), or an operator (`+`, `=`).

**Example:**

The code `int x = 10;` would be converted into the following token stream:

`[KEYWORD:int]`, `[IDENTIFIER:x]`, `[OPERATOR:=]`, `[INTEGER:10]`, `[PUNCTUATION:;]`

This token stream is then passed to the parser for the next stage.

## 2. Syntax Analysis (Parser)

*   **Component:** `Parser`
*   **Source Files:** `src/parser.cpp`, `include/parser.hpp`

**Responsibility:** The parser takes the stream of tokens from the lexer and verifies that it conforms to the grammatical rules of the Nytrogen language (as defined in `docs/grammer.md`). As it analyzes the tokens, the parser builds an **Abstract Syntax Tree (AST)**.

The AST is a tree-like data structure that represents the syntactic structure of the source code in a hierarchical way. It captures the essential relationships between different parts of the code, making it easier for subsequent phases to analyze.

## 3. Semantic Analysis

*   **Component:** `SemanticAnalyzer`
*   **Source Files:** `src/semantic_analyzer.cpp`, `include/semantic_analyzer.hpp`

**Responsibility:** The semantic analyzer takes the AST from the parser and checks it for semantic errors. This involves ensuring that the code is not just syntactically correct, but also meaningful.

Key tasks performed by the semantic analyzer include:

*   **Type Checking:** Verifying that operations are performed on compatible data types (e.g., you can't add a string to an integer).
*   **Variable Declaration:** Ensuring that every variable is declared before it is used.
*   **Scope Checking:** Resolving which variable or function is being referred to, based on the current scope.
*   **Function Calls:** Checking that functions are called with the correct number and types of arguments.

The semantic analyzer annotates the AST with type information and other details, which are then used by the code generator.

## 4. Code Generation

*   **Component:** `CodeGenerator`
*   **Source Files:** `src/code_generator.cpp`, `include/code_generator.hpp`

**Responsibility:** The final phase of the compiler is the code generator. It takes the semantically verified AST and translates it into low-level code. In the case of the Nytrogen compiler, this would typically be assembly code or machine code for a specific target architecture.

The code generator traverses the AST and emits the corresponding instructions for each node. This process involves:

*   **Instruction Selection:** Choosing the appropriate machine instructions for each operation.
*   **Register Allocation:** Deciding which variables to store in CPU registers for faster access.
*   **Memory Management:** Generating code to allocate and deallocate memory for variables and data structures.

## Conclusion

This modular architecture makes the Nytrogen compiler easier to develop, test, and maintain. Each phase can be worked on independently, as long as it adheres to the expected inputs and outputs. This separation of concerns is a fundamental principle in modern compiler design.
