# Nytrogen Compiler

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/yourusername/nytrogen/actions/workflows/build.yml/badge.svg)](https://github.com/yourusername/nytrogen/actions)
[![GitHub Issues](https://img.shields.io/github/issues/yourusername/nytrogen)](https://github.com/yourusername/nytrogen/issues)

Nytrogen is an experimental compiler for a minimalistic programming language, designed to explore the core concepts of compiler design and implementation. It transforms Nytrogen source code (.ny files) into optimized x86-64 assembly, ready for compilation on Linux systems.

## 🚀 Quick Start

```bash
# Clone the repository
git clone https://github.com/yourusername/nytrogen.git
cd nytrogen

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
- **Basic Syntax**
  - Integer literals (e.g., `42`)
  - Variable declarations (`let x = 0;`)
  - Variable assignments (`x = 10;`)
  - Return statements (`return 42;`)

### Planned Features
- Control flow (if/else, while)
- Functions and procedures
- Advanced data types
- Standard library integration
- Error handling and debugging tools
- Arithmetic Operations
  - Addition (`+`)
  - Subtraction (`-`)
  - Multiplication (`*`)
  - Division (`/`)

## 🛠️ Installation

### Prerequisites

Nytrogen requires the following dependencies:

- **C++ Compiler** (g++ recommended)
- **CMake** (version 3.20 or higher)
- **NASM** (Netwide Assembler)
- **GNU Binutils** (for ld)

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

1. **Clone the repository**
```bash
git clone https://github.com/X12-Cloud/nytrogen.git
cd nytrogen
```

2. **Build the compiler**
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

1. **Compile a Nytrogen program**
```bash
./build/Nytro path/to/program.nyt
```
This generates `out.asm` in the current directory.

2. **Assemble and link**
```bash
nasm -f elf64 out.asm -o out.o
ld out.o -o out
```

3. **Run the program**
```bash
./out
```

### Example Program
```nyt
let a = 10;
let b = 5;
let c = a + b;
c = c * 2;
return c;  // Returns 30
```

## 🤝 Contributing

1. Fork the repository
2. Create your feature branch (`git checkout -b feature/AmazingFeature`)
3. Commit your changes (`git commit -m 'feat: Add some AmazingFeature'`)
4. Push to the branch (`git push origin feature/AmazingFeature`)
5. Open a Pull Request

## 📄 License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details.

## 🙏 Acknowledgments

- Inspired by compiler design courses and projects
- Built with C++ and modern development tools
- Thanks to the open-source community for inspiration and resources

## 📞 Support

For support, please:
- Open an issue on GitHub
- Check existing issues for similar problems
- Provide detailed information about your issue

## 📮 Contact

- GitHub: [X12-Cloud](https://github.com/X12-Cloud)
- Email: X12Cloud@gmail.com

## 📜 Documentation

For more detailed documentation, please refer to the [docs](docs) directory.
