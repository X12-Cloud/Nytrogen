

# Getting Started with Nytrogen

This guide will walk you through the process of installing or building the Nytrogen compiler on your local machine, and running your first Nytrogen program.


> [!NOTE]
> Nytrogen has only been thoroughly tested on Arch Linux so far. If you encounter issues on other distributions, please open an issue on GitHub!

## Installation on Arch Linux (AUR)

If you are on Arch Linux (the platform Nytrogen was proudly built on!), you can install it instantly via the AUR using your favorite helper:

```bash
yay -S nytrogen-git
```

Then verify the installation:

```bash
nytro -v
```

Once installed via the AUR, the `nytro` compiler command is globally available in your terminal path.

---

## Building from Source

If you want to build Nytrogen manually from the source code, ensure you have the following prerequisites installed:

- **A C++ Compiler:** A modern compiler supporting C++17 (`g++` or `clang`).
- **CMake:** Version 3.10 or higher.
- **Git:** Required to clone the repository.

### Quick Install on Arch Linux:

```bash
sudo pacman -S base-devel gcc cmake git
```

### Quick Install on Debian/Ubuntu:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

### Build Steps:

1. **Clone the repository:**
   
   ```bash
   git clone https://github.com/X12-Cloud/Nytrogen.git Nytrogen
   cd Nytrogen
   ```

2. **Build using the helper script (Recommended):**
   
   ```bash
   ./run.sh -cbuild
   ```
   
   *(This automatically configures CMake, builds the compiler, and packages the standard library).*

3. **Or build manually with CMake:**
   
   ```bash
   cmake -B build
   cmake --build build -j$(nproc)
   ```

---

## Your First Nytrogen Program: "Hello, World!"

Let's create a simple "Hello, World!" program in Nytrogen.

1. **Create a new file** named `hello.ny`:
   
   ```nytrogen
   int main() {
       print "Hello, World!";
       return 0;
   }
   ```

2. **Run the program** using either the helper script or your compiled binary:
   
   ```bash
   ./run.sh hello.ny
   ```
   
   *Or if installed globally via AUR:*
   
   ```bash
   nytro hello.ny
   ```

You should see the following output in your terminal:

```text
Hello, World!
```

Congratulations! You have successfully set up Nytrogen and run your first program. You are now ready to head over to the [Language Grammar Specification](./spec.md) to explore variables, complex numbers, and quantum types.
