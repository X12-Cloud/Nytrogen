#include <iostream>
#include <stdexcept>
#include <string>
#include <utility>
#include <map>
#include <cctype>

#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current_token_index(0) {}

const Token& Parser::peek(size_t offset) const {
    size_t index = current_token_index + offset;
    if (index >= tokens.size()) {
        return tokens.back();
    }
    return tokens[current_token_index + offset];
}

const Token& Parser::consume() {
    if (current_token_index >= tokens.size()) {
        throw std::runtime_error("Parser Error: Cannot consume token after end of file.");
    }
    return tokens[current_token_index++];
}

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

std::unique_ptr<IntegerLiteralExpressionNode> Parser::parseIntegerLiteralExpression() {
    const Token& int_token = peek();
    expect(Token::INTEGER_LITERAL, "Expected an integer literal.");
    int value = std::stoi(int_token.value);
    return std::make_unique<IntegerLiteralExpressionNode>(value, int_token.line, int_token.column);
}

std::unique_ptr<StringLiteralExpressionNode> Parser::parseStringLiteralExpression() {
    const Token& str_token = peek();
    expect(Token::STRING_LITERAL, "Expected a string literal.");
    return std::make_unique<StringLiteralExpressionNode>(str_token.value, str_token.line, str_token.column);
}

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    const Token& return_token = peek();
    expect(Token::KEYWORD_RETURN, "Expected 'return' keyword.");
    auto expr_node = parseExpression();
    expect(Token::SEMICOLON, "Expected ';' after return expression.");
    return std::make_unique<ReturnStatementNode>(std::move(expr_node), return_token.line, return_token.column);
}

std::unique_ptr<PrintStatementNode> Parser::parsePrintStatement() {
    const Token& print_token = peek();
    expect(Token::KEYWORD_PRINT, "Expected 'print' keyword.");
    auto expr_node = parseExpression();
    expect(Token::SEMICOLON, "Expected ';' after return expression.");
    return std::make_unique<PrintStatementNode>(std::move(expr_node), print_token.line, print_token.column);
}

std::unique_ptr<IfStatementNode> Parser::parseIfStatement() {
    const Token& if_token = peek();
    expect(Token::KEYWORD_IF, "Expected 'if' keyword.");
    expect(Token::LPAREN, "Expected '(' after 'if'.");

    auto condition = parseExpression();

    expect(Token::RPAREN, "Expected ')' after if condition.");
    expect(Token::LBRACE, "Expected '{' to begin 'if' block.");

    std::vector<std::unique_ptr<ASTNode>> true_block;

    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        true_block.push_back(parseStatement());
    }

    expect(Token::RBRACE, "Expected '}' to close 'if' block.");

    std::vector<std::unique_ptr<ASTNode>> false_block;

    if (peek().type == Token::KEYWORD_ELSE) {
        consume();
        expect(Token::LBRACE, "Expected '{' to begin 'else' block.");
        while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
            false_block.push_back(parseStatement());
        }
        expect(Token::RBRACE, "Expected '}' to close 'else' block.");
    }

    return std::make_unique<IfStatementNode>(
        std::move(condition),
        std::move(true_block),
        std::move(false_block),
        if_token.line, if_token.column
    );
}

std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclaration() {
    const Token& type_token = peek();
    if (type_token.type != Token::KEYWORD_INT && type_token.type != Token::KEYWORD_STRING) {
        throw std::runtime_error("Expected 'int' or 'string' keyword for variable declaration.");
    }

    consume();

    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name after type.");

    expect(Token::SEMICOLON, "Expected ';' after variable declaration.");

    return std::make_unique<VariableDeclarationNode>(id_token.value, type_token.type, id_token.line, id_token.column);
}

std::unique_ptr<VariableAssignmentNode> Parser::parseVariableAssignment() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");
    expect(Token::EQ, "Expected '=' after variable name.");
    std::unique_ptr<ASTNode> expr_node;

    if (peek().type == Token::STRING_LITERAL) {
        expr_node = parseStringLiteralExpression();
    } else {
        expr_node = parseExpression();
    }
    expect(Token::SEMICOLON, "Expected ';' after assignment.");
    return std::make_unique<VariableAssignmentNode>(id_token.value, std::move(expr_node), id_token.line, id_token.column);
}

std::unique_ptr<VariableReferenceNode> Parser::parseVariableReference() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");
    return std::make_unique<VariableReferenceNode>(id_token.value, id_token.line, id_token.column);
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    const Token& current_token = peek();
    if (current_token.type == Token::INTEGER_LITERAL) {
        return parseIntegerLiteralExpression();
    } else if (current_token.type == Token::IDENTIFIER) {
        return parseVariableReference();
    } else if (current_token.type == Token::LPAREN) {
        consume();
        auto expr = parseExpression();
        expect(Token::RPAREN, "Expected ')' after expression in parentheses.");
        return expr;
    } else if (current_token.type == Token::STRING_LITERAL) {
        return parseStringLiteralExpression();
    }
    throw std::runtime_error("Parser Error: Expected an integer literal, identifier, or '(' for an expression factor. Got '" +
                             current_token.value + "' at line " + std::to_string(current_token.line) +
                             ", column " + std::to_string(current_token.column) + ".");
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left_expr = parseFactor();

    while (peek().type == Token::STAR || peek().type == Token::SLASH) {
        const Token& op_token = consume();
        auto right_expr = parseFactor();
        left_expr = std::make_unique<BinaryOperationExpressionNode>(
            std::move(left_expr), op_token.type, std::move(right_expr),
            op_token.line, op_token.column
        );
    }
    return left_expr;
}

std::unique_ptr<ASTNode> Parser::parseAdditiveExpression() {
    auto left = parseTerm();

    while (peek().type == Token::PLUS || peek().type == Token::MINUS) {
        const Token& op_token = consume();
        auto right = parseTerm();
        left = std::make_unique<BinaryOperationExpressionNode>(
            std::move(left), op_token.type, std::move(right),
            op_token.line, op_token.column
        );
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseComparisonExpression() {
    auto left = parseAdditiveExpression();

    while (peek().type == Token::EQUAL_EQUAL || peek().type == Token::BANG_EQUAL ||
           peek().type == Token::LESS || peek().type == Token::LESS_EQUAL ||
           peek().type == Token::GREATER || peek().type == Token::GREATER_EQUAL) {
        const Token& op_token = consume();
        auto right = parseAdditiveExpression();
        left = std::make_unique<BinaryOperationExpressionNode>(
            std::move(left), op_token.type, std::move(right),
            op_token.line, op_token.column
        );
    }

    return left;
}

std::unique_ptr<ASTNode> Parser::parseExpression() {
    return parseComparisonExpression();
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    switch (peek().type) {
        case Token::KEYWORD_RETURN:
            return parseReturnStatement();
        case Token::KEYWORD_INT:
        case Token::KEYWORD_STRING:
            return parseVariableDeclaration();
        case Token::IDENTIFIER:
            return parseVariableAssignment();
        case Token::KEYWORD_PRINT:
            return parsePrintStatement();
        case Token::KEYWORD_IF:
            return parseIfStatement();
        default:
            throw std::runtime_error("Parser Error: Unexpected token in statement: '" +
                                     peek().value + "' at line " + std::to_string(peek().line) +
                                     ", column " + std::to_string(peek().column));
    }
}

std::unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
    const Token& return_type_token = peek();
    expect(Token::KEYWORD_INT, "Expected function return type (e.g., 'int').");

    const Token& function_name_token = peek();
    expect(Token::IDENTIFIER, "Expected function name.");

    expect(Token::LPAREN, "Expected '(' after function name.");
    expect(Token::RPAREN, "Expected ')' after function parameters.");

    expect(Token::LBRACE, "Expected '{' to begin function body.");

    auto func_def_node = std::make_unique<FunctionDefinitionNode>(
        return_type_token.type, function_name_token.value,
        return_type_token.line, return_type_token.column
    );

    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        func_def_node->body_statements.push_back(parseStatement());
    }

    expect(Token::RBRACE, "Expected '}' to end function body.");

    return func_def_node;
}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>();

    while (peek().type != Token::END_OF_FILE) {
        try {
            auto func_def = parseFunctionDefinition();
            program_node->functions.push_back(std::move(func_def));
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

