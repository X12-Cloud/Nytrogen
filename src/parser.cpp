#include <iostream>
#include <stdexcept>
#include <string>
#include <utility> // For std::move
#include <map>     // For std::map (declared_variables)
#include <cctype>  // For std::isdigit, std::isalpha, etc.

#include "parser.hpp"
#include "lexer.hpp"
#include "ast.hpp"

// Initialize parser with token stream
Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current_token_index(0) {}

// Look ahead in token stream without consuming
const Token& Parser::peek(size_t offset) const {
    size_t index = current_token_index + offset;
    if (index >= tokens.size()) {
        return tokens.back(); // Should always be END_OF_FILE
    }
    return tokens[current_token_index + offset];
}

// Consume and return current token
const Token& Parser::consume() {
    if(current_token_index >= tokens.size()) {
        throw std::runtime_error("Parser Error: Cannot consume token after end of file.");
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

// Parse variable references
std::unique_ptr<VariableReferenceNode> Parser::parseVariableReference() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");
    return std::make_unique<VariableReferenceNode>(id_token.value, id_token.line, id_token.column);
}

// Parses the most basic parts of an expression: literals, variables, or parenthesized expressions.
std::unique_ptr<ASTNode> Parser::parseFactor() {
    const Token& current_token = peek();
    if (current_token.type == Token::INTEGER_LITERAL) {
        return parseIntegerLiteralExpression();
    } else if (current_token.type == Token::IDENTIFIER) {
        return parseVariableReference();
    } else if (current_token.type == Token::LPAREN) {
        consume(); // Consume '('
        auto expr = parseExpression(); // Recursively parse the expression inside
        expect(Token::RPAREN, "Expected ')' after expression in parentheses.");
        return expr;
    }
    throw std::runtime_error("Parser Error: Expected an integer literal, identifier, or '(' for an expression factor. Got '" +
                             current_token.value + "' at line " + std::to_string(current_token.line) +
                             ", column " + std::to_string(current_token.column) + ".");
}

// Parses multiplication and division operations.
std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left_expr = parseFactor(); // Start with a factor

    while (peek().type == Token::STAR || peek().type == Token::SLASH) {
        const Token& op_token = consume(); // Consume '*' or '/'
        auto right_expr = parseFactor();   // Parse the right-hand side factor
        left_expr = std::make_unique<BinaryOperationExpressionNode>(
            std::move(left_expr), op_token.type, std::move(right_expr),
            op_token.line, op_token.column
        );
    }
    return left_expr;
}

// Parses addition and subtraction operations. This is the main expression parsing entry point.
std::unique_ptr<ASTNode> Parser::parseExpression() {
    auto left_expr = parseTerm(); // Start with a term

    while (peek().type == Token::PLUS || peek().type == Token::MINUS) {
        const Token& op_token = consume(); // Consume '+' or '-'
        auto right_expr = parseTerm();     // Parse the right-hand side term
        left_expr = std::make_unique<BinaryOperationExpressionNode>(
            std::move(left_expr), op_token.type, std::move(right_expr),
            op_token.line, op_token.column
        );
    }
    return left_expr;
}

// Parse a function definition (e.g., 'int main() { ... }')
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

    // Parse statements within the function body until '}'
    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        if (peek().type == Token::KEYWORD_RETURN) {
            auto return_stmt = parseReturnStatement();
            func_def_node->body_statements.push_back(std::move(return_stmt));
        } else if (peek().type == Token::KEYWORD_INT) {
            auto var_decl = parseVariableDeclaration();
            func_def_node->body_statements.push_back(std::move(var_decl));
        } else if (peek().type == Token::IDENTIFIER) {
            auto var_assign = parseVariableAssignment();
            func_def_node->body_statements.push_back(std::move(var_assign));
        } else {
            throw std::runtime_error("Parser Error: Expected statement inside function body. Got '" +
                                     peek().value + "' at line " + std::to_string(peek().line) +
                                     ", column " + std::to_string(peek().column) + ".");
        }
    }

    expect(Token::RBRACE, "Expected '}' to end function body.");

    return func_def_node;
}

// Main parsing function
std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>();

    // A Nytrogen program now consists of one or more function definitions
    while (peek().type != Token::END_OF_FILE) {
        try {
            // Assume top-level constructs are always function definitions for now
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
