# Nytrogen Language Grammar

This document provides a detailed specification of the Nytrogen programming language grammar. It is intended for developers who want to write Nytrogen programs or contribute to the compiler itself.

## Program Structure

A Nytrogen program consists of a series of declarations, which can be either struct definitions or function definitions. The program's execution begins at the `main` function.

```
Program ::= (StructDefinition | FunctionDefinition)*
```

## Comments

Comments are used to add explanatory notes to the code and are ignored by the compiler. Nytrogen supports single-line comments starting with `//`.

```nytrogen
// This is a single-line comment.
int x = 10; // This comment is at the end of a line.
```

## Data Types

Nytrogen supports a range of fundamental data types:

*   `int`: A 32-bit signed integer.
*   `string`: A sequence of characters enclosed in double quotes.
*   `bool`: A boolean value, which can be `true` or `false`.
*   `char`: A single character enclosed in single quotes.
*   `void`: Represents the absence of a value, typically used as a function return type.

### Pointers

You can create a pointer to a variable by using the `*` symbol:

```nytrogen
int* ptr; // Declares a pointer to an integer.
```

### Arrays

Arrays are fixed-size collections of elements of the same type:

```nytrogen
int numbers[10]; // Declares an array of 10 integers.
```

## Variables

Variables are used to store and manipulate data. They must be declared with a specific type before they can be used.

### Declaration

A variable declaration specifies the type and name of the variable.

```nytrogen
// Declaring variables of different types
int count;
bool is_active;
string message;
```

### Initialization

You can initialize a variable at the time of declaration:

```nytrogen
int score = 100;
bool is_finished = false;
string name = "Nytrogen";
```

### Assignment

After a variable has been declared, you can assign it a new value:

```nytrogen
score = 150;
is_finished = true;
```

## Structs

Structs allow you to create complex data types by grouping together variables of different types.

### Definition

Here is how you define a struct:

```nytrogen
struct Point {
    int x;
    int y;
};
```

### Usage

Once a struct is defined, you can declare variables of that type:

```nytrogen
Point p1;
p1.x = 10;
p1.y = 20;
```

## Functions

Functions are blocks of code that can be defined and called to perform a specific task.

### Definition

A function definition includes a return type, a name, a list of parameters, and a body.

```nytrogen
// A function that adds two integers
int add(int a, int b) {
    return a + b;
}

// A function that does not return a value
void print_message(string msg) {
    print msg;
}
```

### The `main` Function

The `main` function is the entry point of every Nytrogen program. It is where the execution of the program begins.

*   **Return Type:** The `main` function must have a return type of `int`.
*   **Parameters:** It takes no parameters.

```nytrogen
int main() {
    // Your program's code goes here
    print "Hello from main!";
    return 0; // A return value of 0 indicates success
}
```

## Control Flow

Nytrogen provides several control flow statements to manage the execution path of your program.

### `if-else` Statement

The `if-else` statement allows you to execute different blocks of code based on a condition.

```nytrogen
int x = 10;
if (x > 0) {
    print "Positive";
} else {
    print "Not positive";
}
```

### `while` Loop

The `while` loop repeatedly executes a block of code as long as a condition is true.

```nytrogen
int i = 0;
while (i < 5) {
    print i;
    i = i + 1;
}
```

### `for` Loop

The `for` loop is ideal for iterating a specific number of times.

```nytrogen
for (int i = 0; i < 5; i = i + 1) {
    print i;
}
```

## Expressions

Expressions are combinations of values, variables, and operators that are evaluated to produce a new value.

*   **Arithmetic:** `+`, `-`, `*`, `/`
*   **Comparison:** `==`, `!=`, `<`, `>`, `<=`, `>=`
*   **Logical:** `&&` (AND), `||` (OR), `!` (NOT)

## Built-in Functions

Nytrogen provides a `print` function for displaying output.

### `print`

The `print` function can output the value of any expression.

```nytrogen
print "Hello, Nytrogen!";
int version = 1;
print version;
```
