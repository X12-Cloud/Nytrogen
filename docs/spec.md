

# Nytrogen Language Specification


This document provides a detailed specification of the Nytrogen programming language. It is intended for developers who want to write Nytrogen programs or contribute to the compiler itself.


## Program Structure

A Nytrogen program consists of a series of declarations, which can include struct definitions, namespace definitions, and function definitions. The program's execution strictly begins at the `main` function.

`Program ::= (StructDefinition | FunctionDefinition | NamespaceDefinition)*`

## Comments

Comments are used to add explanatory notes to the code and are ignored by the compiler. Nytrogen supports single-line comments starting with `//`.

```nytrogen
// This is a single-line comment.
int x = 10; // This comment is at the end of a line.
```

## Data Types

Nytrogen supports a rich set of fundamental and specialized system data types:

- `int`: A 32-bit signed integer.
- `long`: A 64-bit signed integer.
- `float`: A 32-bit decimal number.
- `double`: A 64-bit decimal number.
- `string`: A sequence of characters enclosed in double quotes.
- `bool`: A boolean value, which can be `true` or `false`.
- `char`: A single character enclosed in single quotes.
- `complex`: A complex number containing real and imaginary components.
- `qubit`: A quantum register primitive.
- `matrix`: A multi-dimensional container used for quantum gate matrices.

### Pointers

You can create a pointer to a variable by using the `*` symbol:

```nytrogen
int* ptr; // Declares a pointer to an integer.
```

### Arrays

Arrays are fixed-size collections of elements of the same type:

```nytrogen
int numbers[10]; // Declares an array of 10 integers.
numbers[3] = 20; // Assigns 20 to the 4th element.
print numbers[3];
```

## Variables

Variables are used to store and manipulate data. They must be declared with a specific type (or deduced via `auto`) before they can be used.

### Declaration & Initialization

```nytrogen
int count = 5;
long big_num = 9223372036854775807l;
bool is_active = true;
string message = "Nytrogen 0.133";
```

### Assignment

```nytrogen
count = 10;
```

## Structs

Structs allow you to create custom complex data types by grouping together variables of different types.

### Definition & Usage

```nytrogen
struct Point {
    int x;
    int y;
};

Point p1;
p1.x = 10;
p1.y = 20;
```

## Namespaces

Namespaces allow you to scope functions, variables, and structures cleanly to avoid collisions.

```nytrogen
namespace MathLib {
    struct Vector {
        int x;
        int y;
    };
}
MathLib::Vector v;
```

## Functions

Functions are blocks of code defined to perform specific tasks. Nytrogen supports both traditional return types and modern trailing return types using `auto`.

### Definition

```nytrogen
// Traditional function syntax
int add(int a, int b) {
    return a + b;
}

// Modern trailing return type syntax
auto multiply(int a, int b) -> int {
    return a * b;
}
```

### The `main` Function

The entry point of every Nytrogen program. It must return an `int` and take no parameters.

```nytrogen
int main() {
    print "Hello from main!";
    return 0;
}
```

## Quantum Programming (Qlib Integration)

Nytrogen features native syntax primitives for quantum computing simulation through `qubit` definitions and gate operations.

```nytrogen
// Initialize a qubit with custom amplitudes alpha and beta
qubit q(a: 1, b: 0);

// Apply a quantum gate via arrow operations
h -> q;
print q; // Print the raw qubit amplitudes in a table
print (int)q; // Measure the qubit (gives a random 1 or 0 when in superposition)
```

## Control Flow

### `if-else` Statement

```nytrogen
int x = 10;
if (x > 0) {
    print "Positive";
} else {
    print "Not positive";
}
```

### `while` Loop

```nytrogen
int i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

### `for` Loop

```nytrogen
for (int i = 0; i < 5; i = i + 1) {
    print i;
}
```

### `switch` Statement

```nytrogen
int choice = 2;
switch (choice) {
    case 1:
        print "One";
        break;
    case 2:
        print "Two";
        break;
    default:
        print "Other";
        break;
}
```

## Extern (FFI)

`extern` allows seamless foreign function interface integration with standard C binaries.

```nytrogen
extern malloc(int size);

int ptr = malloc(64);
```

## Built-in Functions & I/O

### `print`

The `print` function handles output displays and supports stream redirection using `to`.

```nytrogen
print "Hello, Nytrogen!";
print "An error occurred!" to std::err;
```

### `format`

The `format` function allows powerful string interpolation using `{}` placeholders.

```nytrogen
string name = "X_12";
string greeting = format("Hello, {}!" : name);
print greeting;
```

---

For the formal EBNF grammar specification head to [Nytrogen EBNF Grammar Specification](./grammar.md)
