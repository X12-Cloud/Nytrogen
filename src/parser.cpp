#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <map>

#include "parser.hpp"
#include "lexer.hpp"

// Parser constructor
#include <iostream>
#include <stdexcept>
#include <string>
#include <cctype>
#include "parser.hpp"
#include "lexer.hpp"

// Initialize parser with token stream
Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current_token_index(0) {}

// Look ahead in token stream without consuming
const Token& Parser::peek(size_t offset) const {
    size_t index = current_token_index + offset;
    if (index >= tokens.size()) {
        return tokens.back();
    }
    return tokens[current_token_index + offset];
}

// Consume and return current token
const Token& Parser::consume() {
    if(current_token_index >= tokens.size()) {
        throw std::runtime_error("Parser Error: Cant consume after the end of tokens.");
    }
    return tokens[current_token_index++];
}

// Verify token type matches expected or throw error
void Parser::expect(Token::Type expected_type, const std::string& error_msg) {
    const Token& current_token = peek();
    if (current_token.type != expected_type) {
        throw std::runtime_error(error_msg +
            " (Got " + current_token.typeToString() +
            " '" + current_token.value +
            "' at line " + std::to_string(current_token.line) +
            ", column " + std::to_string(current_token.column) + ")");
    } else {
        consume();
    }
}

// Parse integer literals
std::unique_ptr<IntegerLiteralExpressionNode> Parser::parseIntegerLiteralExpression() {
    const Token& int_token = peek();
    expect(Token::INTEGER_LITERAL, "Expected an integer literal.");

    int value = std::stoi(int_token.value);
    return std::make_unique<IntegerLiteralExpressionNode>(value, int_token.line, int_token.column);
}

// Parse return statements
std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    const Token& return_token = peek();
    expect(Token::KEYWORD_RETURN, "Expected 'return' keyword.");

    auto expr_node = parseExpression();

    expect(Token::SEMICOLON, "Expected ';' after return expression.");

    return std::make_unique<ReturnStatementNode>(std::move(expr_node), return_token.line, return_token.column);
}

// Parse variable declarations
std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclaration() {
    const Token& int_token = peek();
    expect(Token::KEYWORD_INT, "Expected 'int' keyword.");

    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name after 'int'.");

    expect(Token::SEMICOLON, "Expected ';' after variable declaration.");

    return std::make_unique<VariableDeclarationNode>(id_token.value, id_token.line, id_token.column);
}

// Parse variable assignments
std::unique_ptr<VariableAssignmentNode> Parser::parseVariableAssignment() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");

    expect(Token::EQ, "Expected '=' after variable name.");

    auto expr_node = parseExpression();

    expect(Token::SEMICOLON, "Expected ';' after assignment.");

    return std::make_unique<VariableAssignmentNode>(id_token.value, std::move(expr_node), id_token.line, id_token.column);
}

// Parse expressions
std::unique_ptr<ASTNode> Parser::parseExpression() {
    if (peek().type == Token::INTEGER_LITERAL) {
        return parseIntegerLiteralExpression();
    } else if (peek().type == Token::IDENTIFIER) {
        return parseVariableReference();
    }
    throw std::runtime_error("Expected expression.");
}

// Parse variable references
std::unique_ptr<VariableReferenceNode> Parser::parseVariableReference() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");
    return std::make_unique<VariableReferenceNode>(id_token.value, id_token.line, id_token.column);
}

// Main parsing function
std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>();

    while (peek().type != Token::END_OF_FILE) {
        try {
            if (peek().type == Token::KEYWORD_RETURN) {
                auto return_stmt = parseReturnStatement();
                program_node->statements.push_back(std::move(return_stmt));
            } else if (peek().type == Token::KEYWORD_INT) {
                auto var_decl = parseVariableDeclaration();
                program_node->statements.push_back(std::move(var_decl));
            } else if (peek().type == Token::IDENTIFIER) {
                auto var_assign = parseVariableAssignment();
                program_node->statements.push_back(std::move(var_assign));
            } else {
                std::cerr << "Parser Error: Expected statement. Got '" << peek().value << "' at line " << peek().line << ", column " << peek().column << std::endl;
                return nullptr;
            }
        } catch (const std::runtime_error& e) {
            std::cerr << "Parsing failed: " << e.what() << std::endl;
            return nullptr;
        }
    }

    try {
        expect(Token::END_OF_FILE, "Expected end of file after parsing program. Unexpected tokens found.");
    } catch (const std::runtime_error& e) {
        std::cerr << "Parsing failed at end of file check: " << e.what() << std::endl;
        return nullptr;
    }
    
    return program_node;
}