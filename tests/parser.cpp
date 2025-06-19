#include <iostream>
#include <stdexcept>
#include <string>
#include <cctype>
#include "parser.hpp"
#include "lexer.hpp"

Parser::Parser(std::vector<Token> tokens) : tokens(std::move(tokens)), current_token_index(0) {}

std::unique_ptr<ProgramNode> Parser::parse() {
    return parseProgram();
}

const Token& Parser::peek(size_t offset) const {
    size_t index = current_token_index + offset;
    if (index >= tokens.size()) {
        //throw std::runtime_error("Parser Error: Attempted to peek past end of token stream");
        return tokens.back();
    }
    return tokens[current_token_index + offset];
}

//----------------COME-BACK-HERE----------------//
const Token& Parser::consume() {
    if(current_token_index >= tokens.size) {
        throw std::runtime_error("Parser Error: Cant consume after the end of tokens")
    } else {
        // const Token& consumed_token = tokens[current_token_index];
        // current_token_index++;
        // return consumed_token
        return tokens[current_token_index++];
    }
}

void Parser::expect(Token::Type expected_type, const std::string& error_msg) {
    const Token& current_token = parse();
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

std::unique_ptr<IntegerLiteralExpressionNode> Parser::parseIntegerLiteralExpression {
    const Token& int_token = peek();
    expect(Token::INTEGER_LITERAL, "Expected an integer literal.");

    int value = std::stio(current_token.value);
    std::make_unique<IntegerLiteralExpressionNode>(value, line, column);
}

std::unique_ptr<ReturnStatementNode> Parser::parseReturnStatement() {
    const Token& return_token = peek();
    expect(Token::RETURN_STATEMENT, "use return");

    auto expr_node = parseIntegerLiteralExpression();

    expect(Token::SEMICOLON, "every statement should end with -> ;");

    return std::make_unique<ReturnStatementNode>(std::move(expr_node), return_token.line, return_token.column);
}

//failed att :/
// std::unique_ptr<ProgramNode> Parser::parse() {
//     std::make_unique<ProgrmaNode>()

//     const Token& current_token = peek();
//     if (current_token = Token::ReturnStatement) {
//         auto return_node = parseReturnStatement();
//         parse_tree.push_back(std::move(return_node));
//     } else if (current_token != Token::END_OF_FILE) {
//         throw std::runtime_error("Parser Error: Expected a return statement or end of file");
//         delete return_node;
//         return_node = nullptr;
//     }

//     expect(Token::END_OF_FILE, "Expected end of file");
//     return std::make_unique<ProgramNode>(std::move(parse_tree));
// }

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>(); // Create the root node of the AST

    // In this simple compiler, we only expect ONE return statement.
    // We check if the current token is 'return'.
    if (peek().type == Token::KEYWORD_RETURN) {
        try {
            // If it is 'return', parse it as a ReturnStatement and add it to the program's statements
            auto return_stmt = parseReturnStatement();
            program_node->statements.push_back(std::move(return_stmt)); // std::move transfers ownership
        } catch (const std::runtime_error& e) {
            // If parsing the return statement fails, catch the error, print it, and return nullptr
            // This allows main.cpp to know parsing failed.
            std::cerr << "Parsing failed during statement processing: " << e.what() << std::endl;
            return nullptr; // Indicate a fatal parsing error
        }
    } else if (peek().type != Token::END_OF_FILE) {
        // If the first token is neither 'return' nor 'END_OF_FILE', it's an error.
        // For example, if the file starts with "int x = 5;" or "unknown_keyword;".
        std::cerr << "Parser Error: Expected 'return' keyword or end of file. Got '"
                  << peek().value << "' at line " << peek().line << ", column " << peek().column << std::endl;
        return nullptr; // Indicate a fatal parsing error
    }

    // After parsing all expected statements (in our case, potentially one return statement),
    // ensure we are exactly at the end of the token stream.
    // If there's extra code after the return statement, this will catch it.
    try {
        expect(Token::END_OF_FILE, "Expected end of file after parsing program. Unexpected tokens found.");
    } catch (const std::runtime_error& e) {
        std::cerr << "Parsing failed at end of file check: " << e.what() << std::endl;
        return nullptr; // Indicate a fatal parsing error
    }
    
    return program_node; // Return the completed AST (the root ProgramNode)
}