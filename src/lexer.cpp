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
        case KEYWORD_INT: return "KEYWORD_INT";
	case KEYWORD_STRING: return "KEYWORD_STRING";
        case IDENTIFIER: return "IDENTIFIER";
        case EQ: return "EQ";
	case PLUS: return "PLUS";
	case MINUS: return "MINUS";
 	case STAR: return "STAR";
	case SLASH: return "SLASH";
        case SEMICOLON: return "SEMICOLON";
	case LPAREN: return "LPAREN";
	case RPAREN: return "RPAREN";
	case LBRACE: return "LBRACE";
	case RBRACE: return "RBRACE";
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

        // --- Single-line comment detection ---
	if (currentChar == '/' && currentPos + 1 < sourceCode.length() && sourceCode[currentPos + 1] == '/') {
    	// Skip everything until newline
    	currentPos += 2;
    	column += 2;
    	while (currentPos < sourceCode.length() && sourceCode[currentPos] != '\n') {
        	currentPos++;
        	column++;
    	}
    	continue;
	}


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

	if (currentChar == '"') {
            std::string value;
            int startColumn = column;
            currentPos++; // Consume the opening '"'
            column++;

            while (currentPos < sourceCode.length() && sourceCode[currentPos] != '"') {
                value += sourceCode[currentPos];
                currentPos++;
                column++;
            }

            if (currentPos >= sourceCode.length()) {
                std::cerr << "Lexer Error: Unclosed string literal starting at line " << line << ", column " << startColumn << std::endl;
            } else {
                currentPos++; // Consume the closing '"'
                column++;
            }
            tokens.push_back({Token::STRING_LITERAL, value, line, startColumn});
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
            } else if (value == "string") {
                tokens.push_back({Token::KEYWORD_STRING, value, line, startColumn});
	    } else if (value == "print") { 
		tokens.push_back({Token::KEYWORD_PRINT, value, line, startColumn});
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

 	if (currentChar == '+') {
            tokens.push_back({Token::PLUS, "+", line, column});
            currentPos++;
            column++;
            continue;
        }

	if (currentChar == '-') {
            tokens.push_back({Token::MINUS, "-", line, column});
            currentPos++;
            column++;
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
    }

    tokens.push_back({Token::END_OF_FILE, "", line, column});
    return tokens;
}
