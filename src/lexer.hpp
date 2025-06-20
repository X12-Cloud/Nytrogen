#ifndef NYTROGEN_LEXER_HPP
#define NYTROGEN_LEXER_HPP

#include <string>
#include <vector>

// Represents a single token in the source code
struct Token {
    enum Type {
        KEYWORD_RETURN,
	KEYWORD_INT,
        IDENTIFIER,
        EQ,
        INTEGER_LITERAL,
        SEMICOLON,
        WHITESPACE,
        END_OF_FILE,
        UNKNOWN
    };

    Type type;
    std::string value;
    int line;
    int column;

    std::string typeToString() const;
};

// Converts source code into a sequence of tokens
std::vector<Token> tokenize(const std::string& sourceCode);

#endif
