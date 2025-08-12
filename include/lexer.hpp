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
        KEYWORD_VOID,
	KEYWORD_WHILE,
	KEYWORD_BOOL,
	KEYWORD_CHAR,
	KEYWORD_FOR,
	KEYWORD_STRUCT,

        IDENTIFIER,
        INTEGER_LITERAL,
        STRING_LITERAL,
	TRUE,
	FALSE,
	CHARACTER_LITERAL,

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
	ADDRESSOF,	// &

        SEMICOLON,      // ;
        LPAREN,         // (
        RPAREN,         // )
        LBRACE,         // {
        RBRACE,         // }
	LBRACKET,	// [
	RBRACKET,	// ]
	DOT,		// .

        COMMA,          // ,

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
