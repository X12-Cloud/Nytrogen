#ifndef LEXER_HPP
#define LEXER_HPP

#include <string>
#include <vector>

struct Token {
    enum Type {
        KEYWORD_RETURN,
        KEYWORD_PRINT,
        KEYWORD_INT,
        KEYWORD_STRING,
        KEYWORD_IF,
        KEYWORD_ELSE,

        IDENTIFIER,
        INTEGER_LITERAL,
        STRING_LITERAL,

        EQ,             // =
        EQUAL_EQUAL,    // ==
        BANG_EQUAL,     // !=
        LESS,           // <
        GREATER,        // >
        LESS_EQUAL,     // <=
        GREATER_EQUAL,  // >=

        PLUS,           // +
        MINUS,          // -
        STAR,           // *
        SLASH,          // /

        SEMICOLON,      // ;
        LPAREN,         // (
        RPAREN,         // )
        LBRACE,         // {
        RBRACE,         // }

        END_OF_FILE,
        UNKNOWN
    };

    Type type;
    std::string value;
    int line;
    int column;

    std::string typeToString() const;
};

std::vector<Token> tokenize(const std::string& sourceCode);

#endif // LEXER_HPP

