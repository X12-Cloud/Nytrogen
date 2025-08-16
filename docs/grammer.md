# Nytrogen Language Grammar Specification

This document outlines the current grammar of the Nytrogen programming language (v0.1). It defines the valid structure for code written in .ny or .nyt files.

---

## Program Structure

A Nytrogen program is a collection of function definitions and optional struct definitions.

```
Program → (StructDefinition | FunctionDefinition)* EOF
```

---

## Data Types

The language supports the following data types:
- `int`: A 32-bit integer.
- `int*`: A pointer to an integer.
- `int[size]`: An array of integers.
- `struct`: A user-defined data structure.

---

## Variables

Variables must be declared before they are used.

**Declaration:**
```
VariableDeclaration → Type Identifier (";" | "[" IntegerLiteral "]" ";")
```

**Examples:**
```c
int my_variable;
int* my_pointer;
int my_array[10];
```

**Assignment:**
```
VariableAssignment → Identifier "=" Expression ";"
```

**Example:**
```c
my_variable = 100;
```

---

## Structs

Structs are user-defined types that can hold multiple fields.

**Definition:**
```
StructDefinition → "struct" Identifier "{" (VariableDeclaration)+ "}" ";"
```

**Example:**
```c
struct Point {
    int x;
    int y;
};
```

---

## Functions

Functions are defined with a return type, a name, and optional parameters.

**Definition:**
```
FunctionDefinition → Type Identifier "(" (ParameterList)? ")" "{" (Statement)* "}"
```

- The `main` function is the entry point of the program and must be defined.

**Example:**
```c
int add(int a, int b) {
    return a + b;
}

int main() {
    int result;
    result = add(5, 3);
    return 0;
}
```

---

## Statements

Statements are the building blocks of a function.

- **Variable Declaration & Assignment:** (see above)
- **Return Statement:** `return Expression ";"`
- **Print Statement:** `print Expression ";"`
- **Control Flow Statements:** `for` and `while` loops.

---

## Print Statement

The `print` statement is used to output data to the console.

**Syntax:**
```
PrintStatement → "print" Expression ";"
```

**Important Limitations:**
- You can print a single integer expression.
- You can print a single string literal.
- **You cannot combine strings and variables in a single print statement.**

**Valid Examples:**
```c
int x;
x = 10;
print(x);

print("Hello, World!");
```

**Invalid Example:**
```c
// This will not work!
int x;
x = 10;
print("The value is: ", x);
```

---

## Control Flow

**For Loop:**
```
for "(" Statement Statement Expression ")" "{" (Statement)* "}"
```

**Example:**
```c
int i;
for (i = 0; i < 5; i = i + 1) {
    print(i);
}
```

**While Loop:**
```
while "(" Expression ")" "{" (Statement)* "}"
```

**Example:**
```c
int i;
i = 0;
while (i < 5) {
    print(i);
    i = i + 1;
}
```

---

## Expressions

Expressions evaluate to a value.

```
Expression → Term (("+" | "-") Term)*
Term       → Factor (("*" | "/") Factor)*
Factor     → IntegerLiteral
           | StringLiteral
           | Identifier
           | "(" Expression ")"
```

---

Last updated: August 2025