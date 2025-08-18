# Nytrogen Compiler

**Nytrogen** is an educational compiler built to demonstrate the core principles of compiler design. It takes source code written in the Nytrogen language and translates it into executable code.

This project is ideal for students, hobbyists, or anyone interested in learning how programming languages are processed and understood by a computer. It provides a clear and practical example of a compiler pipeline, from lexical analysis to code generation.

## About Nytrogen

*   **Purpose:** Educational tool for learning compiler design.
*   **Language:** A simple, C-style procedural language.
*   **Status:** Under active development.

## Documentation

This project includes detailed documentation to help you get started and understand the compiler's design:

*   **[Getting Started](./docs/getting_started.md):** A comprehensive guide to building and running the Nytrogen compiler.
*   **[Language Grammar](./docs/grammer.md):** A detailed specification of the Nytrogen language syntax and features.
*   **[Compiler Architecture](./docs/architecture.md):** An overview of the internal design of the compiler.

## Features

The Nytrogen language supports a variety of essential programming features:

*   **Data Types:** `int`, `string`, `bool`, `char`, pointers (`*`), and arrays (`[]`).
*   **Control Flow:** `if-else` statements, `for` loops, and `while` loops.
*   **Functions:** Define functions with parameters and return values.
*   **Structs:** Create custom data structures.
*   **Expressions:** Arithmetic, comparison, and logical operators.

For a complete list of features and syntax, please refer to the [Language Grammar](./docs/grammer.md) document.

## Getting Started

To get started with Nytrogen, please see the **[Getting Started Guide](./docs/getting_started.md)**. This guide provides step-by-step instructions for setting up your environment, building the compiler, and running your first program.

In short, you can build and run the compiler using the following scripts:

```bash
# Build the compiler
./build.sh

# Run a Nytrogen source file
./run.sh test.nyt
```

## Project Structure

*   `src/`: Source code for the compiler's components.
*   `include/`: Header files for the compiler.
*   `docs/`: Project documentation.
*   `tests/`: Sample Nytrogen programs for testing.
*   `build.sh`, `run.sh`: Helper scripts for building and running.

## Contributing

Contributions are highly encouraged! If you would like to contribute, please feel free to open an issue or submit a pull request.

## License

This project is licensed under the MIT License. See the [LICENSE](./LICENSE) file for details.