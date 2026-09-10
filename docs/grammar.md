# Nytrogen Language EBNF Grammar Specification

This document provides the formal EBNF Grammar Specification for the Nytrogen programming language.

## Lexical Elements & Notation Conventions

- `::=` : Defined as
- `|` : Alternative (OR)
- `*` : Zero or more repetitions
- `?` : Optional (zero or one)

```ebnf
Program             ::= ( StructDefinition | FunctionDefinition | NamespaceDefinition | ExternDeclaration )*

(* --- Lexical Elements --- *)
Identifier          ::= [a-zA-Z_] [a-zA-Z0-9_]*
IntegerLiteral      ::= [0-9]+
LongLiteral         ::= [0-9]+ "l"
FloatLiteral        ::= [0-9]+ "." [0-9]+ "f"
DoubleLiteral       ::= [0-9]+ "." [0-9]+
StringLiteral       ::= "\"" [^"\n]* "\""
CharLiteral         ::= "'" . "'"

(* --- Types --- *)
Type                ::= PrimitiveType | PointerType | ArrayType | StructType
PrimitiveType       ::= "int" | "long" | "float" | "double" | "string" | "bool" | "char" | "void" | "complex" | "qubit" | "matrix"
PointerType         ::= Type "*"
ArrayType           ::= Type "[" Expression? "]"
StructType          ::= Identifier

(* --- Structs & Namespaces --- *)
StructDefinition    ::= "struct" Identifier "{" StructMember* "}" ";"
StructMember        ::= Type Identifier ";"
NamespaceDefinition ::= "namespace" Identifier "{" ( StructDefinition | FunctionDefinition | VariableDeclaration )* "}"

(* --- Functions & FFI --- *)
FunctionDefinition  ::= ( Type | "auto" ) Identifier "(" ParameterList? ")" ( "->" Type )? Block
ExternDeclaration   ::= "extern" Identifier "(" ParameterList? ")" ";"
ParameterList       ::= Parameter ( "," Parameter )*
Parameter           ::= Type Identifier

(* --- Declarations & Statements --- *)
Statement           ::= VariableDeclaration
                      | QubitDefinition
                      | VariableAssignment
                      | IfStatement
                      | WhileStatement
                      | ForStatement
                      | SwitchStatement
                      | ReturnStatement
                      | PrintStatement
                      | AsmStatement
                      | Block
                      | Expression ";"

VariableDeclaration ::= Type VariableDeclarator ( "," VariableDeclarator )* ";"
VariableDeclarator  ::= Identifier ( "[" Expression "]" )? ( "=" Expression )?
QubitDefinition     ::= "qubit" Identifier ( "(" "a" ":" Expression "," "b" ":" Expression ")" )? ";"
VariableAssignment  ::= Accessor "=" Expression ";"

Block               ::= "{" Statement* "}"

IfStatement         ::= "if" "(" Expression ")" Statement ( "else" Statement )?
WhileStatement      ::= "while" "(" Expression ")" Statement
ForStatement        ::= "for" "(" ( VariableDeclaration | Expression )? ";" Expression? ";" Expression? ")" Statement
SwitchStatement     ::= "switch" "(" Expression ")" "{" SwitchCase* SwitchDefault? "}"
SwitchCase          ::= "case" Expression ":" Statement*
SwitchDefault       ::= "default" ":" Statement*

ReturnStatement     ::= "return" Expression? ";"
AsmStatement        ::= "asm" ( "({ ... })" | "{" Statement* "}" ) ";"

(* --- Built-ins & IO --- *)
PrintStatement      ::= "print" Expression ( "," Expression )* ( "to" Expression )? ";"
FormatExpression    ::= "format" "(" Expression ( ":" Expression )* ")"

(* --- Expressions & Operators --- *)
Expression          ::= AssignmentExpr
AssignmentExpr      ::= LogicalOrExpr ( "=" AssignmentExpr )?
LogicalOrExpr       ::= LogicalAndExpr ( "||" LogicalAndExpr )*
LogicalAndExpr      ::= EqualityExpr ( "&&" EqualityExpr )*
EqualityExpr        ::= RelationalExpr ( ( "==" | "!=" ) RelationalExpr )*
RelationalExpr      ::= AdditiveExpr ( ( "<" | ">" | "<=" | ">=" ) AdditiveExpr )*
AdditiveExpr        ::= MultiplicativeExpr ( ( "+" | "-" ) MultiplicativeExpr )*
MultiplicativeExpr  ::= UnaryExpr ( ( "*" | "/" ) UnaryExpr )*
UnaryExpr           ::= ( "!" | "-" | "*" | "&" | TypeCast ) UnaryExpr | PostfixExpr
TypeCast            ::= "(" Type ")"
PostfixExpr         ::= PrimaryExpr ( "[" Expression "]" | "." Identifier | "(" ArgumentList? ")" | "->" Expression )*

PrimaryExpr         ::= Identifier
                      | IntegerLiteral
                      | LongLiteral
                      | FloatLiteral
                      | DoubleLiteral
                      | StringLiteral
                      | CharLiteral
                      | BooleanLiteral
                      | ComplexLiteral
                      | FormatExpression
                      | "(" Expression ")"

BooleanLiteral      ::= "true" | "false"
ComplexLiteral      ::= FloatLiteral "i" | IntegerLiteral "i" | IntegerLiteral
Accessor            ::= Identifier ( "[" Expression "]" | "." Identifier )*
ArgumentList        ::= Expression ( "," Expression )*
```
