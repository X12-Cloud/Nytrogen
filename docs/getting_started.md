# Getting Started with Nytrogen

This guide will walk you through the process of setting up the Nytrogen compiler on your local machine, building it from the source, and running your first Nytrogen program.

## Prerequisites

Before you begin, ensure you have the following installed on your system:

*   **A C++ Compiler:** You'll need a modern C++ compiler that supports C++17. The most common choice is `g++`, which is part of the GCC (GNU Compiler Collection).
*   **CMake:** CMake is used to automate the build process. You'll need version 3.10 or higher.
*   **Git:** Git is required to clone the project repository.

You can typically install these tools using your system's package manager. For example, on a Debian-based Linux distribution (like Ubuntu), you can run:

```bash
sudo apt-get update
sudo apt-get install build-essential cmake git
```

And on an arch based distribution (Nytrogen was fully made on arch btw), you can run:

```bash
sudo pacman -Syyu # or -Syy
sudo pacman -S base-devel gcc cmake git
```

## Building the Compiler

The Nytrogen compiler can be built using CMake or the provided helper scripts.

### Using the Build Script (Recommended)

The easiest way to build the compiler is to use the `build.sh` script located in the root of the project:

```bash
./build.sh
```

This script will create a `build` directory, run CMake to configure the project, and then compile the source code. The final executable will be placed in the `build` directory.

`Note` You can use run.sh to build or clean build the compiler by passing the arguments
-clean or -build (both would be the same for the first build)

### Manual Build with CMake

If you prefer to build the project manually, follow these steps:

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/your-username/Nytrogen.git
    cd Nytrogen
    ```

2.  **Create a build directory:**
    ```bash
    mkdir build
    cd build
    ```

3.  **Run CMake and compile:**
    ```bash
    cmake ..
    make
    ```

This will generate the `Nytro` executable in the `build` directory.

## Running the Compiler

Once the compiler is built, you can use it to execute Nytrogen source files (`.nyt`).

### Using the Run Script

The `run.sh` script provides a convenient way to compile and run a Nytrogen file. Simply pass the path to your source file as an argument:

```bash
./run.sh <your_source_file.nyt>
```

For example, to run the `test.nyt` file included in the project:

```bash
./run.sh test.nyt
```

### Manual Execution

You can also run the compiler executable directly from the `build` directory:

```bash
./build/Nytro <your_source_file.nyt>
```

## Your First Nytrogen Program: "Hello, World!"

Let's create a simple "Hello, World!" program in Nytrogen.

1.  **Create a new file** named `hello.nyt`:
    ```nytrogen
    int main() {
        print "Hello, World!";
        return 0;
    }
    ```

2.  **Run the program** using the compiler:
    ```bash
    ./run.sh hello.nyt
    ```

You should see the following output in your terminal:

```
Hello, World!
```

Congratulations! You have successfully built and run your first Nytrogen program. You are now ready to explore the language features in more detail.
