#include "lexer.hpp"
#include <iostream>
#include <cctype>

// Token type to string conversion
std::string Token::typeToString() const {
    switch (type) {
        case KEYWORD_RETURN: return "KEYWORD_RETURN";
        case KEYWORD_PRINT: return "KEYWORD_PRINT";
        case INTEGER_LITERAL: return "INTEGER_LITERAL";
        case STRING_LITERAL: return "STRING_LITERAL";
	case TRUE: return "TRUE";
	case FALSE: return "FALSE";
	case CHARACTER_LITERAL: return "CHARACTER_LITERAL";
        case KEYWORD_INT: return "KEYWORD_INT";
        case KEYWORD_VOID: return "KEYWORD_VOID";
        case KEYWORD_STRING: return "KEYWORD_STRING";
        case KEYWORD_IF: return "KEYWORD_IF";
	case KEYWORD_WHILE: return "KEYWORD_WHILE";
        case KEYWORD_ELSE: return "KEYWORD_ELSE";
	case KEYWORD_BOOL: return "KEYWORD_BOOL";
        case KEYWORD_CHAR: return "KEYWORD_CHAR";
        case KEYWORD_FOR: return "KEYWORD_FOR";
	case KEYWORD_CONST: return "KEYWORD_CONST";
        case KEYWORD_STRUCT: return "KEYWORD_STRUCT";
	case KEYWORD_SWITCH: return "KEYWORD_SWITCH";
	case KEYWORD_CASE: return "KEYWORD_CASE";
	case KEYWORD_DEFAULT: return "KEYWORD_DEFAULT";
	case KEYWORD_ASM: return "KEYWORD_ASM";
	case KEYWORD_ENUM: return "KEYWORD_ENUM";
	case KEYWORD_PUBLIC: return "KEYWORD_PUBLIC";
	case KEYWORD_PRIVATE: return "KEYWORD_PRIVATE";

	case IDENTIFIER: return "IDENTIFIER";
        case EQ: return "EQ";
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case STAR: return "STAR";
        case SLASH: return "SLASH";
	case ADDRESSOF: return "ADDRESSOF";
        case EQUAL_EQUAL: return "EQUAL_EQUAL";
        case BANG_EQUAL: return "BANG_EQUAL";
        case LESS: return "LESS";
        case GREATER: return "GREATER";
        case LESS_EQUAL: return "LESS_EQUAL";
        case GREATER_EQUAL: return "GREATER_EQUAL";
        case SEMICOLON: return "SEMICOLON";
        case LPAREN: return "LPAREN";
	case RPAREN: return "RPAREN";
        case LBRACE: return "LBRACE";
        case RBRACE: return "RBRACE";
	case LBRACKET: return "LBRACKET";
        case RBRACKET: return "RBRACKET";
	case COMMA: return "COMMA";
	case DOT: return "DOT";
	case COLON: return "COLON";
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
            while (currentPos < sourceCode.length() &&
                   (std::isalnum(sourceCode[currentPos]) || sourceCode[currentPos] == '_')) {
                value += sourceCode[currentPos++];
                column++;
            }
            if (value == "return") tokens.push_back({Token::KEYWORD_RETURN, value, line, startColumn});
            else if (value == "int") tokens.push_back({Token::KEYWORD_INT, value, line, startColumn});
            else if (value == "string") tokens.push_back({Token::KEYWORD_STRING, value, line, startColumn});
            else if (value == "print") tokens.push_back({Token::KEYWORD_PRINT, value, line, startColumn});
            else if (value == "if") tokens.push_back({Token::KEYWORD_IF, value, line, startColumn});
            else if (value == "else") tokens.push_back({Token::KEYWORD_ELSE, value, line, startColumn});
	    else if (value == "while") tokens.push_back({Token::KEYWORD_WHILE, value, line, startColumn});
	    else if (value == "bool") tokens.push_back({Token::KEYWORD_BOOL, value, line, startColumn});
	    else if (value == "char") tokens.push_back({Token::KEYWORD_CHAR, value, line, startColumn});		
	    else if (value == "true") tokens.push_back({Token::TRUE, value, line, startColumn});
	    else if (value == "false") tokens.push_back({Token::FALSE, value, line, startColumn});
	    else if (value == "for") tokens.push_back({Token::KEYWORD_FOR, value, line, startColumn});
	    else if (value == "struct") tokens.push_back({Token::KEYWORD_STRUCT, value, line, startColumn});
	    else if (value == "switch") tokens.push_back({Token::KEYWORD_SWITCH, value, line, startColumn});
	    else if (value == "case") tokens.push_back({Token::KEYWORD_CASE, value, line, startColumn});
	    else if (value == "default") tokens.push_back({Token::KEYWORD_DEFAULT, value, line, startColumn});
	    else if (value == "asm") tokens.push_back({Token::KEYWORD_ASM, value, line, startColumn});
	    else if (value == "enum") tokens.push_back({Token::KEYWORD_ENUM, value, line, startColumn});
	    else if (value == "const") tokens.push_back({Token::KEYWORD_CONST, value, line, startColumn});
	    else if (value == "public") tokens.push_back({Token::KEYWORD_PUBLIC, value, line, startColumn});
	    else if (value == "private") tokens.push_back({Token::KEYWORD_PRIVATE, value, line, startColumn});
	    else if (value == "extern") tokens.push_back({Token::KEYWORD_EXTERN, value, line, startColumn});
	    else tokens.push_back({Token::IDENTIFIER, value, line, startColumn});
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
