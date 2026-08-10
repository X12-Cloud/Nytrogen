#include "lexer.hpp"

#include <cctype>
#include <iostream>
#include <unordered_map>

static const std::unordered_map<std::string, Token::Type> KEYWORD_MAP = {
    {"return", Token::KEYWORD_RETURN},
    {"int", Token::KEYWORD_INT},
    {"string", Token::KEYWORD_STRING},
    {"print", Token::KEYWORD_PRINT},
    {"if", Token::KEYWORD_IF},
    {"else", Token::KEYWORD_ELSE},
    {"while", Token::KEYWORD_WHILE},
    {"bool", Token::KEYWORD_BOOL},
    {"char", Token::KEYWORD_CHAR},
    {"true", Token::TRUE},
    {"false", Token::FALSE},
    {"for", Token::KEYWORD_FOR},
    {"struct", Token::KEYWORD_STRUCT},
    {"switch", Token::KEYWORD_SWITCH},
    {"case", Token::KEYWORD_CASE},
    {"default", Token::KEYWORD_DEFAULT},
    {"asm", Token::KEYWORD_ASM},
    {"enum", Token::KEYWORD_ENUM},
    {"const", Token::KEYWORD_CONST},
    {"public", Token::KEYWORD_PUBLIC},
    {"private", Token::KEYWORD_PRIVATE},
    {"extern", Token::KEYWORD_EXTERN},
    {"auto", Token::KEYWORD_AUTO},
    {"void", Token::KEYWORD_VOID},
    {"float", Token::KEYWORD_FLOAT},
    {"double", Token::KEYWORD_DOUBLE},
    {"namespace", Token::KEYWORD_NAMESPACE},
    {"complex", Token::KEYWORD_COMPLEX},
    {"matrix", Token::KEYWORD_MATRIX},
    {"qubit", Token::KEYWORD_QUBIT},
    {"format", Token::KEYWORD_FORMAT},
    {"__builtin_", Token::KEYWORD_BUILTIN}};

// Token type to string conversion
std::string Token::typeToString() const {
    static const char* typeStrings[] = {
#define AS_STR(name, str) #name,
        TOKEN_LIST(AS_STR)
#undef AS_STR
    };

    if (this->type >= 0 && this->type < (sizeof(typeStrings) / sizeof(typeStrings[0]))) {
        return typeStrings[this->type];
    }
    return "UNKNOWN";
}

// Lexical analysis - converts source code to tokens
std::vector<Token> tokenize(const std::string& sourceCode) {
    std::vector<Token> tokens;
    int currentPos = 0;
    int line = 1;
    int column = 1;

    while (currentPos < sourceCode.length()) {
        char currentChar = sourceCode[currentPos];

        // Single-line comment
        if (currentChar == '/' && currentPos + 1 < sourceCode.length() &&
            sourceCode[currentPos + 1] == '/') {
            currentPos += 2;
            column += 2;
            while (currentPos < sourceCode.length() && sourceCode[currentPos] != '\n') {
                currentPos++;
                column++;
            }
            continue;
        }

        // Whitespace
        if (std::isspace(currentChar) != 0) {
            if (currentChar == '\n') {
                line++;
                column = 1;
            } else {
                column++;
            }
            currentPos++;
            continue;
        }

        // Any digit literal
        if (std::isdigit(currentChar) != 0) {
            std::string value;
            int startColumn = column;

            // Helper lambda to scan a numeric part (digits and dots)
            auto scanNumber = [&](std::string& buffer) {
                while (currentPos < sourceCode.length() &&
                       (std::isdigit(sourceCode[currentPos]) || sourceCode[currentPos] == '.')) {
                    buffer += sourceCode[currentPos++];
                    column++;
                }
            };

            // Scan the first part (The Real part or the start of a pure imaginary)
            scanNumber(value);

            // Handle optional suffixes for the first part (f, d)
            if (currentPos < sourceCode.length() && (sourceCode[currentPos] == 'f' || sourceCode[currentPos] == 'd')) {
                value += sourceCode[currentPos++];
                column++;
            }

            // COMPLEX CHECK: Look ahead for [+-][number]i
            bool isComplex = false;
            if (currentPos < sourceCode.length() && (sourceCode[currentPos] == '+' || sourceCode[currentPos] == '-')) {
                size_t peekPos = currentPos + 1;
                while (peekPos < sourceCode.length() && (std::isdigit(sourceCode[peekPos]) || sourceCode[peekPos] == '.')) {
                    peekPos++;
                }

                if (peekPos < sourceCode.length() && sourceCode[peekPos] == 'i') {
                    isComplex = true;
                    value += sourceCode[currentPos++];
                    column++;
                    scanNumber(value);
                    value += sourceCode[currentPos++];
                    column++;
                }
            } 
            // Handle pure imaginary case: e.g., "2.0i"
            else if (currentPos < sourceCode.length() && sourceCode[currentPos] == 'i') {
                isComplex = true;
                value += sourceCode[currentPos++];
                column++;
            }

            // Token Generation
            if (isComplex) {
                tokens.push_back({Token::COMPLEX_LITERAL, value, line, startColumn});
            } else if (value.find('.') != std::string::npos) {
                if (value.back() == 'f') {
                    tokens.push_back({Token::FLOAT_LITERAL, value, line, startColumn});
                } else {
                    tokens.push_back({Token::DOUBLE_LITERAL, value, line, startColumn});
                }
            } else {
                tokens.push_back({Token::INTEGER_LITERAL, value, line, startColumn});
            }
            continue;
        }

        // Character literal
        if (currentChar == '\'') {
            std::string value;
            int startColumn = column;
            currentPos++;
            column++;  // Skip initial quote
            if (currentPos < sourceCode.length()) {
                value += sourceCode[currentPos++];
                column++;
            }
            if (currentPos >= sourceCode.length() || sourceCode[currentPos] != '\'') {
                std::cerr << "Lexer Error: Unclosed or invalid character literal at line " << line
                          << ", column " << startColumn << std::endl;
            } else {
                currentPos++;
                column++;  // Skip closing quote
            }
            tokens.push_back({Token::CHARACTER_LITERAL, value, line, startColumn});
            continue;
        }

        // String literal
        if (currentChar == '"') {
            std::string value;
            int startColumn = column;
            currentPos++;
            column++;  // Skip initial quote
            while (currentPos < sourceCode.length() && sourceCode[currentPos] != '"') {
                value += sourceCode[currentPos++];
                column++;
            }
            if (currentPos >= sourceCode.length()) {
                std::cerr << "Lexer Error: Unclosed string literal at line " << line << ", column "
                          << startColumn << std::endl;
            } else {
                currentPos++;
                column++;  // Skip closing quote
            }
            tokens.push_back({Token::STRING_LITERAL, value, line, startColumn});
            continue;
        }

        // Identifier or keyword
        if ((std::isalpha(currentChar) != 0) || currentChar == '_') {
            std::string value;
            int startColumn = column;
            while (currentPos < sourceCode.length() &&
                   ((std::isalnum(sourceCode[currentPos]) != 0) || sourceCode[currentPos] == '_')) {
                value += sourceCode[currentPos++];
                column++;
            }

            auto it = KEYWORD_MAP.find(value);
            if (it != KEYWORD_MAP.end()) {
                tokens.push_back({it->second, value, line, startColumn});
            } else {
                tokens.push_back({Token::IDENTIFIER, value, line, startColumn});
            }
            continue;
        }

        // == or =
        if (currentChar == '=') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::EQUAL_EQUAL, "==", line, column});
                currentPos += 2;
                column += 2;
            } else {
                tokens.push_back({Token::EQ, "=", line, column});
                currentPos++;
                column++;
            }
            continue;
        }

        // !=
        if (currentChar == '!') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::BANG_EQUAL, "!=", line, column});
                currentPos += 2;
                column += 2;
            } else {
                tokens.push_back({Token::BANG, "!", line, column});
                currentPos++;
                column++;
            }
            continue;
        }

        // < or <=
        if (currentChar == '<') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::LESS_EQUAL, "<=", line, column});
                currentPos += 2;
                column += 2;
            } else {
                tokens.push_back({Token::LESS, "<", line, column});
                currentPos++;
                column++;
            }
            continue;
        }

        // > or >=
        if (currentChar == '>') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::GREATER_EQUAL, ">=", line, column});
                currentPos += 2;
                column += 2;
            } else {
                tokens.push_back({Token::GREATER, ">", line, column});
                currentPos++;
                column++;
            }
            continue;
        }

        // : or ::
        if (currentChar == ':') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == ':') {
                tokens.push_back({Token::DOUBLE_COLON, "::", line, column});
                currentPos += 2;
                column += 2;
            } else {
                tokens.push_back({Token::COLON, ":", line, column});
                currentPos++;
                column++;
            }
            continue;
        }

        // Other single-char tokens
        if (currentChar == ';') {
            tokens.push_back({Token::SEMICOLON, ";", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '+') {
            tokens.push_back({Token::PLUS, "+", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '-') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '>') {
                tokens.push_back({Token::ARROW, "->", line, column});
                currentPos += 2;
                column += 2;
            } else {
                tokens.push_back({Token::MINUS, "-", line, column});
                currentPos++;
                column++;
            }
            continue;
        }

        if (currentChar == '*') {
            tokens.push_back({Token::STAR, "*", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '/') {
            tokens.push_back({Token::SLASH, "/", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '(') {
            tokens.push_back({Token::LPAREN, "(", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == ')') {
            tokens.push_back({Token::RPAREN, ")", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '{') {
            tokens.push_back({Token::LBRACE, "{", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '}') {
            tokens.push_back({Token::RBRACE, "}", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '[') {
            tokens.push_back({Token::LBRACKET, "[", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == ']') {
            tokens.push_back({Token::RBRACKET, "]", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '.') {
            tokens.push_back({Token::DOT, ".", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == ',') {
            tokens.push_back({Token::COMMA, ",", line, column});
            currentPos++;
            column++;
            continue;
        }

        if (currentChar == '&') {
            tokens.push_back({Token::ADDRESSOF, "&", line, column});
            currentPos++;
            column++;
            continue;
        }

        // Unknown character
        std::cerr << "Lexer Error: Unknown character '" << currentChar << "' at line " << line
                  << ", column " << column << std::endl;
        currentPos++;
        column++;
    }

    tokens.push_back({Token::END_OF_FILE, "", line, column});
    return tokens;
}
