#include "lexer.hpp"
#include <iostream>
#include <cctype>

// Token type to string conversion
std::string Token::typeToString() const {
    switch (type) {
        case KEYWORD_RETURN: return "KEYWORD_RETURN";
        case INTEGER_LITERAL: return "INTEGER_LITERAL";
        case KEYWORD_INT: return "KEYWORD_INT";
        case IDENTIFIER: return "IDENTIFIER";
        case EQ: return "EQ";
        case SEMICOLON: return "SEMICOLON";
        case END_OF_FILE: return "END_OF_FILE";
        case UNKNOWN: return "UNKNOWN";
    }
    return "UNKNOWN_TYPE";
}

// Lexical analysis - converts source code to tokens
std::vector<Token> tokenize(const std::string& sourceCode) {
    std::vector<Token> tokens;
    int currentPos = 0;
    int line = 1;
    int column = 1;

    bool inComment = false;
    while (currentPos < sourceCode.length()) {
        char currentChar = sourceCode[currentPos];

        // if (currentChar == '/') {
        //     if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '/') {
        //         inComment = true;
        //         currentPos += 2;
        //         column += 2;
        //         continue;
        //     }
        //     // Single '/' is an unknown character
        //     tokens.push_back({Token::UNKNOWN, std::string(1, currentChar), line, column});
        //     std::cerr << "Lexer Error: Unrecognized character '" << currentChar << "' at line " << line << ", column " << column << std::endl;
        //     currentPos++;
        //     column++;
        //     continue;
        // }
        //
        // if (inComment) {
        //         if (currentChar == '\n') {
        //             inComment = false;
        //             line++;
        //             column = 1;
        //             currentPos++;
        //             continue;
        //         }
        //         currentPos++;
        //         column++;
        //         continue;
        //     }

        if (std::isspace(currentChar)) {
            if (currentChar == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            currentPos++;
            continue;
        }

        if (std::isdigit(currentChar)) {
            std::string value;
            int startColumn = column;
            while (currentPos < sourceCode.length() && std::isdigit(sourceCode[currentPos])) {
                value += sourceCode[currentPos];
                currentPos++;
                column++;
            }
            tokens.push_back({Token::INTEGER_LITERAL, value, line, startColumn});
            continue;
        }

        if (std::isalpha(currentChar) || currentChar == '_') {
            std::string value;
            int startColumn = column;
            while (currentPos < sourceCode.length() &&
                   (std::isalnum(sourceCode[currentPos]) || sourceCode[currentPos] == '_')) {
                value += sourceCode[currentPos];
                currentPos++;
                column++;
            }

            if (value == "return") {
                tokens.push_back({Token::KEYWORD_RETURN, value, line, startColumn});
            } else if (value == "int") {
                tokens.push_back({Token::KEYWORD_INT, value, line, startColumn});
            } else {
                tokens.push_back({Token::IDENTIFIER, value, line, startColumn});
            }
            continue;
        }
        
        if (currentChar == '=') {
            tokens.push_back({Token::EQ, "=", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == ';') {
            tokens.push_back({Token::SEMICOLON, ";", line, column});
            currentPos++;
            column++;
            continue;
        }
    }

    tokens.push_back({Token::END_OF_FILE, "", line, column});
    return tokens;
}
