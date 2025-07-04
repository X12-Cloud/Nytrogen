# Nytrogen Language Grammar Specification

This document outlines the current grammar of the Nytrogen programming language (v0.1). It defines the valid structure for code written in .ny or .nyt files.

---

## Program Structure

Program → (FunctionDefinition)* EOF

---

## Function Definition

FunctionDefinition → "int" Identifier "(" ")" "{" (Statement)* "}"

- Only `int` return types are supported.
- No function parameters yet.
- Must include one `main` function.

---

## Statements

Statement →
    VariableDeclaration
  | VariableAssignment
  | ReturnStatement
  | PrintStatement

---

## Variable Declaration

VariableDeclaration → "int" Identifier ";"

- Declares an integer variable with default value 0.

---

## Variable Assignment

VariableAssignment → Identifier "=" Expression ";"

- Assigns the result of an expression to a variable.

---

## Return Statement

ReturnStatement → "return" Expression ";"

- Ends the function and returns an integer result.

---

## Print Statement

PrintStatement → "print" Expression ";"

- Prints an integer expression to stdout.
- Currently only integer printing is supported.

---

## Expressions

Expression → Term (("+" | "-") Term)*
Term       → Factor (("*" | "/") Factor)*
Factor     → IntegerLiteral
           | StringLiteral (partially supported)
           | Identifier
           | "(" Expression ")"

- Operator precedence is preserved.
- Strings are lexed and parsed but not fully compiled yet.

---

## Literals

IntegerLiteral → [0-9]+  
StringLiteral  → "\"" .*? "\""

---

## Identifiers

Identifier → [a-zA-Z_][a-zA-Z0-9_]*

---

## EOF

EOF → End of input

---

Last updated: July 2025

