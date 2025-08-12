# Nytrogen Compiler

Nytrogen is a simple, educational compiler designed to demonstrate the fundamental concepts of compiler design, including lexical analysis, parsing, and abstract syntax tree (AST) generation. It aims to provide a clear and concise example of how a programming language can be processed and understood by a machine.

## Features

The Nytrogen language currently supports the following features:

### Data Types
*   `int`: Integer numbers.
*   `string`: Sequences of characters.
*   `bool`: Boolean values (`true` and `false`).
*   `char`: Single characters.
*   Pointers (`*`): Support for pointer types.
*   Arrays (`[]`): Support for array types and array access.

### Variables
*   **Declaration:** Declare variables with specified types (e.g., `int x;`, `string message = "Hello";`).
*   **Assignment:** Assign values to declared variables (e.g., `x = 10;`).
*   **Referencing:** Use variables in expressions and statements.

### Control Flow
*   **Conditional Statements:** `if` and `else` constructs for branching logic.
    ```nytrogen
    if (x > 0) {
        print "Positive";
    } else {
        print "Non-positive";
    }
    ```
*   **While Loops:** Execute a block of code repeatedly as long as a condition is true.
    ```nytrogen
    int i = 0;
    while (i < 5) {
        print i;
        i = i + 1;
    }
    ```
*   **For Loops:** Iterate over a sequence with an initializer, condition, and increment.
    ```nytrogen
    for (int i = 0; i < 5; i = i + 1) {
        print i;
    }
    ```

### Functions
*   **Definition:** Define functions with a return type, name, and parameters.
    ```nytrogen
    int add(int a, int b) {
        return a + b;
    }
    ```
*   **Function Calls:** Invoke defined functions with arguments.
    ```nytrogen
    int result = add(5, 3);
    print result;
    ```
*   **Return Statements:** Return a value from a function.

### Expressions
*   **Arithmetic Operations:** Perform addition (`+`), subtraction (`-`), multiplication (`*`), and division (`/`).
*   **Comparison Operations:** Compare values using equality (`==`), inequality (`!=`), less than (`<`), greater than (`>`), less than or equal to (`<=`), and greater than or equal to (`>=`).
*   **Unary Operations:** Support for unary operators like address-of (`&`).
*   **Array Access:** Access elements of an array using an index (e.g., `myArray[0]`).

### Statements
*   **Print Statement:** Output the value of an expression to the console (e.g., `print "Hello World";`, `print myVariable;`).

## Getting Started

### Prerequisites
*   A C++ compiler (e.g., g++).
*   CMake (version 3.10 or higher).

### Building the Compiler

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/Nytrogen.git
    cd Nytrogen
    ```
2.  **Create a build directory and compile:**
    ```bash
    mkdir build
    cd build
    cmake ..
    make
    ```
    Alternatively, you can use the provided `build.sh` script from the project root:
    ```bash
    ./build.sh
    ```

### Running the Compiler

After building, the executable will be located in the `build` directory. You can run it with a Nytrogen source file as an argument:

```bash
./build/nytrogen <your_source_file.nyt>
```

Example:
```bash
./build/nytrogen test.nyt
```

You can also use the `run.sh` script for convenience:
```bash
./run.sh test.nyt
```

## Project Structure

*   `src/`: Contains the source code for the lexer, parser, and main compiler logic.
*   `include/`: Contains header files for AST nodes, lexer, and parser.
*   `build/`: Directory for build artifacts (created by CMake).
*   `docs/`: Documentation files.
*   `test.nyt`: An example Nytrogen source file for testing.
*   `build.sh`: Script to build the project.
*   `run.sh`: Script to run the compiler with a test file.
*   `clean_run.sh`: Script to clean the build and then run.
*   `commit.sh`: Script to commit changes.

## Contributing

Contributions are welcome! Please feel free to open issues or submit pull requests.

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.
