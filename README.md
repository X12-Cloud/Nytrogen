# Nytrogen Compiler

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/X12-Cloud/Nytrogen/actions/workflows/build.yml/badge.svg)](https://github.com/X12-Cloud/Nytrogen/actions)
[![GitHub Issues](https://img.shields.io/github/issues/X12-Cloud/Nytrogen)](https://github.com/X12-Cloud/Nytrogen/issues)

Nytrogen is an experimental compiler for a minimalistic programming language, designed to explore the core concepts of compiler design and implementation. It transforms Nytrogen source code (.ny/.nyt files) into optimized x86-64 assembly, ready for compilation on Linux systems.

## 🚀 Quick Start - Build from source

```bash
# Clone the repository
git clone https://github.com/X12-Cloud/Nytrogen.git
cd Nytrogen

# Build the compiler
mkdir build
cd build
cmake ..
cmake --build .

# Compile a test program
../build/Nytro ../test.nyt

# Assemble and link
nasm -f elf64 out.asm -o out.o
ld out.o -o out

# Run the program
./out
```

## 📚 Language Features

### Current Implementation
Nytrogen supports a growing set of features, focusing on core programming constructs:

*   **Data Types:** `int`, `bool`, `char`, and basic `string` literals.
*   **Variables:** Declarations and assignments for all supported types.
*   **Arithmetic Operations:** Standard integer operations (`+`, `-`, `*`, `/`).
*   **Control Flow:**
    *   Conditional statements (`if`/`else`).
    *   Looping constructs (`while` loops, `for` loops).
*   **Functions:**
    *   User-defined functions with parameters and return values.
    *   Function calls.
*   **Basic I/O:** `print` statement for integers, booleans, characters, and strings.
*   **Memory Operations:** Address-of (`&`) and Dereference (`*`) operators (initial support for pointers).
*   **Arrays:** Basic array declarations and indexing (`arr[index]`).

### Planned Features
- Standard library integration (e.g., file I/O, advanced string manipulation)
- More complex data structures (e.g., structs)
- Error handling and debugging tools

## 🛠️ Installation

### Prerequisites

Nytrogen requires the following dependencies:

-   **C++ Compiler** (g++ recommended)
-   **CMake** (version 3.20 or higher)
-   **NASM** (Netwide Assembler)
-   **GNU Binutils** (for ld)

#### Installation on Ubuntu/Debian
```bash
sudo apt update
sudo apt install build-essential cmake nasm
```

#### Installation on Arch Linux
```bash
sudo pacman -S base-devel cmake nasm
```

### Building from Source

1.  **Clone the repository**
    ```bash
    git clone https://github.com/X12-Cloud/Nytrogen.git
    cd nytrogen
    ```

2.  **Build the compiler**
    ```bash
    mkdir build
    cd build
    cmake ..
    cmake --build .
    ```

The compiler executable will be located at `build/Nytro`.

## 📦 Project Structure

```
nytrogen/
├── build/              # Build artifacts
├── src/                # Source code
│   ├── main.cpp        # Compiler entry point
│   ├── lexer.hpp/cpp   # Lexical analysis
│   ├── parser.hpp/cpp  # Syntax analysis
│   └── ast.hpp         # Abstract Syntax Tree
├── test.nyt            # Example program
├── CMakeLists.txt      # CMake configuration
├── run.sh
├── clean_run.sh
└── run_scripts/        # Build and run scripts
    ├── run.sh
    ├── build.sh
    ├── clean_build.sh
    └── clean_run.sh
```

## 📝 Usage

### Compiling Programs

1.  **Compile a Nytrogen program**
    ```bash
    ./build/Nytro path/to/program.nyt
    ```
    This generates `out.asm` in the current directory.

2.  **Assemble and link**
    ```bash
    nasm -f elf64 out.asm -o out.o
    ld out.o -o out
    ```

3.  **Run the program**
    ```bash
    ./out
    ```

### Example Program
```nyt
// Example demonstrating various features
int add(int a, int b) {
    return a + b;
}

int main() {
    int x = 10;
    int y = 5;
    bool flag = true;
    char initial = 'A';

    print("Hello from Nytrogen!");
    print(x + y); // Prints 15

    if (flag) {
        print("Flag is true!");
    }

    for (int i = 0; i < 3; i = i + 1) {
        print(i);
    }

    int sum = add(x, y);
    print(sum); // Prints 15

    int arr[2];
    arr[0] = 100;
    arr[1] = 200;
    print(arr[0]); // Prints 100

    int* ptr = &x;
    print(*ptr); // Prints 10

    *ptr = 50;
    print(x); // Prints 50

    return 0;
}
```

## 🤝 Contributing

1.  Fork the repository
2.  Create your feature branch (`git checkout -b feature/AmazingFeature`)
3.  Commit your changes (`git commit -m 'feat: Add some AmazingFeature'`)
4.  Push to the branch (`git push origin feature/AmazingFeature`)
5.  Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

-   Inspired by compiler design courses and projects
-   Built with C++ and modern development tools
-   Thanks to the open-source community for inspiration and resources

## 📞 Support

For support, please:
-   Open an issue on GitHub
-   Check existing issues for similar problems
-   Provide detailed information about your issue

## 📮 Contact

-   GitHub: [X12-Cloud](https://github.com/X12-Cloud)
-   Email: X12Cloud@gmail.com

## 📜 Documentation

For more detailed documentation, please refer to the [docs](docs) directory.