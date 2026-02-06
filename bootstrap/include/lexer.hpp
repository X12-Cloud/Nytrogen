#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

// THE MASTER LIST: X(EnumName, StringValue)
// Keywords use their string value, Operators/Literals use a label
#define TOKEN_LIST(X) \
    X(KEYWORD_RETURN, "return")   X(KEYWORD_PRINT, "print")     \
    X(KEYWORD_INT, "int")         X(KEYWORD_STRING, "string")   \
    X(KEYWORD_IF, "if")           X(KEYWORD_ELSE, "else")       \
    X(KEYWORD_VOID, "void")       X(KEYWORD_WHILE, "while")     \
    X(KEYWORD_BOOL, "bool")       X(KEYWORD_CHAR, "char")       \
    X(KEYWORD_FOR, "for")         X(KEYWORD_CONST, "const")     \
    X(KEYWORD_STRUCT, "struct")   X(KEYWORD_SWITCH, "switch")   \
    X(KEYWORD_CASE, "case")       X(KEYWORD_DEFAULT, "default") \
    X(KEYWORD_ASM, "asm")         X(KEYWORD_ENUM, "enum")       \
    X(KEYWORD_PUBLIC, "public")   X(KEYWORD_PRIVATE, "private") \
    X(KEYWORD_EXTERN, "extern")   X(KEYWORD_AUTO, "auto")       \
    X(IDENTIFIER, "ID")           X(INTEGER_LITERAL, "INT_LIT") \
    X(STRING_LITERAL, "STR_LIT")  X(TRUE, "true")               \
    X(FALSE, "false")             X(CHARACTER_LITERAL, "CHAR_LIT") \
    X(EQ, "=")                    X(EQUAL_EQUAL, "==")          \
    X(BANG_EQUAL, "!=")           X(LESS, "<")                  \
    X(GREATER, ">")               X(LESS_EQUAL, "<=")           \
    X(GREATER_EQUAL, ">=")        X(PLUS, "+")                  \
    X(MINUS, "-")                 X(STAR, "*")                  \
    X(SLASH, "/")                 X(ADDRESSOF, "&")             \
    X(SEMICOLON, ";")             X(LPAREN, "(")                \
    X(RPAREN, ")")                X(LBRACE, "{")                \
    X(RBRACE, "}")                X(LBRACKET, "[")              \
    X(RBRACKET, "]")              X(DOT, ".")                   \
    X(COLON, ":")                 X(COMMA, ",")                 \
    X(END_OF_FILE, "EOF")         X(UNKNOWN, "UNKNOWN")

struct Token {
    enum Type {
#define AS_ENUM(name, str) name,
        TOKEN_LIST(AS_ENUM)
#undef AS_ENUM
    };

    Type type;
    std::string value;
    int line;
    int column;

    std::string typeToString() const;
};

std::vector<Token> tokenize(const std::string& sourceCode);

#endif
