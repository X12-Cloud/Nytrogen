#include "lexer.hpp"
#include <iostream>
#include <cctype>
#include <unordered_map>

static const std::unordered_map<std::string, Token::Type> KEYWORD_MAP = {
    {"return", Token::KEYWORD_RETURN}, {"int", Token::KEYWORD_INT},
    {"string", Token::KEYWORD_STRING}, {"print", Token::KEYWORD_PRINT},
    {"if", Token::KEYWORD_IF},         {"else", Token::KEYWORD_ELSE},
    {"while", Token::KEYWORD_WHILE},   {"bool", Token::KEYWORD_BOOL},
    {"char", Token::KEYWORD_CHAR},     {"true", Token::TRUE},
    {"false", Token::FALSE},           {"for", Token::KEYWORD_FOR},
    {"struct", Token::KEYWORD_STRUCT}, {"switch", Token::KEYWORD_SWITCH},
    {"case", Token::KEYWORD_CASE},     {"default", Token::KEYWORD_DEFAULT},
    {"asm", Token::KEYWORD_ASM},       {"enum", Token::KEYWORD_ENUM},
    {"const", Token::KEYWORD_CONST},   {"public", Token::KEYWORD_PUBLIC},
    {"private", Token::KEYWORD_PRIVATE}, {"extern", Token::KEYWORD_EXTERN},
    {"auto", Token::KEYWORD_AUTO},     {"void", Token::KEYWORD_VOID}
};

// Token type to string conversion
// Token type to string conversion
std::string Token::typeToString() const {
    static const char* typeStrings[] = {
#define AS_STR(name, str) #name,
        TOKEN_LIST(AS_STR)
#undef AS_STR
    };

    // Since this->type is an enum (integer), we just index the array
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
        if (currentChar == '/' && currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '/') {
            currentPos += 2;
            column += 2;
            while (currentPos < sourceCode.length() && sourceCode[currentPos] != '\n') {
                currentPos++;
                column++;
            }
            continue;
        }

        // Whitespace
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

        // Integer literal
        if (std::isdigit(currentChar)) {
            std::string value;
            int startColumn = column;
            while (currentPos < sourceCode.length() && std::isdigit(sourceCode[currentPos])) {
                value += sourceCode[currentPos++];
                column++;
            }
            tokens.push_back({Token::INTEGER_LITERAL, value, line, startColumn});
            continue;
        }

        // Character literal
        if (currentChar == '\'') {
            std::string value;
            int startColumn = column;
            currentPos++; column++; // Skip initial quote
            if (currentPos < sourceCode.length()) {
                value += sourceCode[currentPos++];
                column++;
            }
            if (currentPos >= sourceCode.length() || sourceCode[currentPos] != '\'') {
                std::cerr << "Lexer Error: Unclosed or invalid character literal at line " << line << ", column " << startColumn << std::endl;
            } else {
                currentPos++; column++; // Skip closing quote
            }
            tokens.push_back({Token::CHARACTER_LITERAL, value, line, startColumn});
            continue;
        }

        // String literal
        if (currentChar == '"') {
            std::string value;
            int startColumn = column;
            currentPos++; column++; // Skip initial quote
            while (currentPos < sourceCode.length() && sourceCode[currentPos] != '"') {
                value += sourceCode[currentPos++];
                column++;
            }
            if (currentPos >= sourceCode.length()) {
                std::cerr << "Lexer Error: Unclosed string literal at line " << line << ", column " << startColumn << std::endl;
            }
            else {
                currentPos++; column++; // Skip closing quote
            }
            tokens.push_back({Token::STRING_LITERAL, value, line, startColumn});
            continue;
        }

        // Identifier or keyword
	if (std::isalpha(currentChar) || currentChar == '_') {
    	    std::string value;
    	    int startColumn = column;
    	    while (currentPos < sourceCode.length() && (std::isalnum(sourceCode[currentPos]) || sourceCode[currentPos] == '_')) {
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
                currentPos += 2; column += 2;
            } else {
                tokens.push_back({Token::EQ, "=", line, column});
                currentPos++; column++;
            }
            continue;
        }

        // !=
        if (currentChar == '!') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::BANG_EQUAL, "!=", line, column});
                currentPos += 2; column += 2;
            } else {
                std::cerr << "Lexer Error: Unexpected character '!' at line " << line << ", column " << column << std::endl;
                currentPos++; column++;
            }
            continue;
        }

        // < or <=
        if (currentChar == '<') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::LESS_EQUAL, "<=", line, column});
                currentPos += 2; column += 2;
            } else {
                tokens.push_back({Token::LESS, "<", line, column});
                currentPos++; column++;
            }
            continue;
        }

        // > or >=
        if (currentChar == '>') {
            if (currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '=') {
                tokens.push_back({Token::GREATER_EQUAL, ">=", line, column});
                currentPos += 2; column += 2;
            } else {
                tokens.push_back({Token::GREATER, ">", line, column});
                currentPos++; column++;
            }
            continue;
        }

        // Other single-char tokens
        if (currentChar == ';') {
            tokens.push_back({Token::SEMICOLON, ";", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '+') {
            tokens.push_back({Token::PLUS, "+", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '-') {
            tokens.push_back({Token::MINUS, "-", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '*') {
            tokens.push_back({Token::STAR, "*", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '/') {
            tokens.push_back({Token::SLASH, "/", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '(') {
            tokens.push_back({Token::LPAREN, "(", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == ')') {
            tokens.push_back({Token::RPAREN, ")", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '{') {
            tokens.push_back({Token::LBRACE, "{", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '}') {
            tokens.push_back({Token::RBRACE, "}", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '[') {
            tokens.push_back({Token::LBRACKET, "[", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == ']') {
            tokens.push_back({Token::RBRACKET, "]", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '.') {
            tokens.push_back({Token::DOT, ".", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == ':') {
            tokens.push_back({Token::COLON, ":", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == ',') {
            tokens.push_back({Token::COMMA, ",", line, column});
            currentPos++; column++; continue;
        }

        if (currentChar == '&') {
            tokens.push_back({Token::ADDRESSOF, "&", line, column});
            currentPos++; column++; continue;
        }

        // Unknown character
        std::cerr << "Lexer Error: Unknown character '" << currentChar << "' at line " << line << ", column " << column << std::endl;
        currentPos++; column++;
    }

    tokens.push_back({Token::END_OF_FILE, "", line, column});
    return tokens;
}
