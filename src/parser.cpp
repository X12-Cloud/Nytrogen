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

std::unique_ptr<BooleanLiteralExpressionNode> Parser::parseBooleanLiteralExpression() {
    const Token& bool_token = peek();
    if (bool_token.type == Token::TRUE) {
        consume();
        return std::make_unique<BooleanLiteralExpressionNode>(true, bool_token.line, bool_token.column);
    } else if (bool_token.type == Token::FALSE) {
        consume();
        return std::make_unique<BooleanLiteralExpressionNode>(false, bool_token.line, bool_token.column);
    }
    throw std::runtime_error("Expected 'true' or 'false' literal.");
}

std::unique_ptr<CharacterLiteralExpressionNode> Parser::parseCharacterLiteralExpression() {
    const Token& char_token = peek();
    expect(Token::CHARACTER_LITERAL, "Expected a character literal.");
    return std::make_unique<CharacterLiteralExpressionNode>(char_token.value[0], char_token.line, char_token.column);
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

std::unique_ptr<WhileStatementNode> Parser::parseWhileStatement() {
    const Token& while_token = peek();
    expect(Token::KEYWORD_WHILE, "Expected 'while' keyword.");
    expect(Token::LPAREN, "Expected '(' after 'while'.");

    auto condition = parseExpression();

    expect(Token::RPAREN, "Expected ')' after while condition.");
    expect(Token::LBRACE, "Expected '{' to begin 'while' block.");

    std::vector<std::unique_ptr<ASTNode>> body;
    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        body.push_back(parseStatement());
    }

    expect(Token::RBRACE, "Expected '}' to close 'while' block.");

    return std::make_unique<WhileStatementNode>(
        std::move(condition),
        std::move(body),
        while_token.line, while_token.column
    );
}

    std::unique_ptr<ForStatementNode> Parser::parseForStatement() {
    const Token& for_token = peek();
    expect(Token::KEYWORD_FOR, "Expected 'for' keyword.");
    expect(Token::LPAREN, "Expected '(' after 'for'.");

    std::unique_ptr<ASTNode> initializer = nullptr;
    // Parse initializer (optional)
    if (peek().type != Token::SEMICOLON) {
        // If it starts with a type keyword, it's a declaration
        if (peek().type == Token::KEYWORD_INT || peek().type == Token::KEYWORD_STRING ||
            peek().type == Token::KEYWORD_BOOL || peek().type == Token::KEYWORD_CHAR) {
            initializer = parseVariableDeclaration();
        } else {
            // Otherwise, it's an expression (assignment or function call)
            initializer = parseExpression();
        }
    }
    expect(Token::SEMICOLON, "Expected ';' after for loop initializer.");

    std::unique_ptr<ASTNode> condition = nullptr;
    // Parse condition (optional)
    if (peek().type != Token::SEMICOLON) {
        condition = parseExpression();
    }
    expect(Token::SEMICOLON, "Expected ';' after for loop condition.");

    std::unique_ptr<ASTNode> increment = nullptr;
    // Parse increment (optional)
    if (peek().type != Token::RPAREN) {
        // The increment part can be an expression or an assignment
        if (peek().type == Token::IDENTIFIER && peek(1).type == Token::EQ) {
            increment = parseVariableAssignment();
        } else {
            increment = parseExpression();
        }
    }
    expect(Token::RPAREN, "Expected ')' after for loop increment.");

    expect(Token::LBRACE, "Expected '{' to begin 'for' block.");

    std::vector<std::unique_ptr<ASTNode>> body;
    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        body.push_back(parseStatement());
    }

    expect(Token::RBRACE, "Expected '}' to close 'for' block.");

    return std::make_unique<ForStatementNode>(
        std::move(initializer),
        std::move(condition),
        std::move(increment),
        std::move(body),
        for_token.line, for_token.column
    );
}

std::unique_ptr<TypeNode> Parser::parseType() {
    const Token& type_token = peek();
    std::unique_ptr<TypeNode> type;

    if (type_token.type == Token::KEYWORD_INT ||
        type_token.type == Token::KEYWORD_STRING ||
        type_token.type == Token::KEYWORD_BOOL ||
        type_token.type == Token::KEYWORD_CHAR) {
        consume();
        type = std::make_unique<PrimitiveTypeNode>(type_token.type);
    } else if (type_token.type == Token::IDENTIFIER) {
        // Check if it's a defined struct
        if (symbol_table.isStructDefined(type_token.value)) {
            consume();
            type = std::make_unique<StructTypeNode>(type_token.value);
        } else {
            throw std::runtime_error("Expected 'int', 'string', 'bool', 'char', or a defined struct name for type.");
        }
    } else {
        throw std::runtime_error("Expected 'int', 'string', 'bool', 'char', or a defined struct name for type.");
    }

    while (peek().type == Token::STAR) {
        consume();
        type = std::make_unique<PointerTypeNode>(std::move(type));
    }

    // Handle array types (e.g., int[] or int*[5])
    if (peek().type == Token::LBRACKET) {
        consume(); // Consume '['
        int array_size = -1; // -1 indicates unsized array or size not specified yet
        if (peek().type == Token::INTEGER_LITERAL) {
            array_size = std::stoi(consume().value);
        }
        expect(Token::RBRACKET, "Expected ']' after array size.");
        type = std::make_unique<ArrayTypeNode>(std::move(type), array_size);
    }

    return type;
}

std::unique_ptr<VariableDeclarationNode> Parser::parseVariableDeclaration() {
    auto type = parseType();

    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name after type.");

    if (peek().type == Token::LBRACKET) {
        consume(); // Consume '['
        const Token& size_token = peek();
        expect(Token::INTEGER_LITERAL, "Expected integer literal for array size.");
        int size = std::stoi(size_token.value);
        expect(Token::RBRACKET, "Expected ']' after array size.");
        type = std::make_unique<ArrayTypeNode>(std::move(type), size);
    }

    std::unique_ptr<ASTNode> initial_value = nullptr;
    if (peek().type == Token::EQ) {
        consume(); // Consume '='
        initial_value = parseExpression();
    }

    // Add variable to symbol table
    symbol_table.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, id_token.value, type->clone(), 0, 0));

    return std::make_unique<VariableDeclarationNode>(id_token.value, std::move(type), std::move(initial_value), id_token.line, id_token.column);
}

std::unique_ptr<VariableAssignmentNode> Parser::parseVariableAssignment() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");

    if (peek().type == Token::LBRACKET) {
        consume(); // consume '['
        auto index_expr = parseExpression();
        expect(Token::RBRACKET, "Expected ']' after array index.");
        expect(Token::EQ, "Expected '=' after array access.");
        auto expr_node = parseExpression();
        return std::make_unique<VariableAssignmentNode>(id_token.value, std::move(expr_node), std::move(index_expr), id_token.line, id_token.column);
    }

    expect(Token::EQ, "Expected '=' after variable name.");
    auto expr_node = parseExpression();
    return std::make_unique<VariableAssignmentNode>(id_token.value, std::move(expr_node), nullptr, id_token.line, id_token.column);
}

std::unique_ptr<VariableReferenceNode> Parser::parseVariableReference() {
    const Token& id_token = peek();
    expect(Token::IDENTIFIER, "Expected variable name.");
    return std::make_unique<VariableReferenceNode>(id_token.value, id_token.line, id_token.column);
}

std::unique_ptr<FunctionCallNode> Parser::parseFunctionCall() {
    const Token& id_token = consume(); // Consume the function name identifier

    expect(Token::LPAREN, "Expected '(' after function name for a function call.");

    std::vector<std::unique_ptr<ASTNode>> arguments;
    if (peek().type != Token::RPAREN) {
        arguments.push_back(parseExpression());
        while (peek().type == Token::COMMA) {
            consume(); // Consume the comma
            arguments.push_back(parseExpression());
        }
    }

    expect(Token::RPAREN, "Expected ')' after function call arguments.");

    return std::make_unique<FunctionCallNode>(id_token.value, std::move(arguments), id_token.line, id_token.column);
}

std::unique_ptr<ASTNode> Parser::parseFactor() {
    std::unique_ptr<ASTNode> node;
    const Token& current_token = peek();

    if (current_token.type == Token::INTEGER_LITERAL) {
        node = parseIntegerLiteralExpression();
    } else if (current_token.type == Token::IDENTIFIER) {
        if (peek(1).type == Token::LPAREN) {
            node = parseFunctionCall();
        } else if (peek(1).type == Token::LBRACKET) {
            auto var_ref = parseVariableReference();
            consume(); // consume '['
            auto index_expr = parseExpression();
            expect(Token::RBRACKET, "Expected ']' after array index.");
            node = std::make_unique<ArrayAccessNode>(std::move(var_ref), std::move(index_expr));
        } else {
            node = parseVariableReference();
        }
    } else if (current_token.type == Token::LPAREN) {
        consume();
        node = parseExpression();
        expect(Token::RPAREN, "Expected ')' after expression in parentheses.");
    } else if (current_token.type == Token::STRING_LITERAL) {
        node = parseStringLiteralExpression();
    } else if (current_token.type == Token::TRUE || current_token.type == Token::FALSE) {
        node = parseBooleanLiteralExpression();
    } else if (current_token.type == Token::CHARACTER_LITERAL) {
        node = parseCharacterLiteralExpression();
    } else {
        throw std::runtime_error("Parser Error: Expected an integer literal, identifier, or '(' for an expression factor. Got '" +
                                 current_token.value + "' at line " + std::to_string(current_token.line) +
                                 ", column " + std::to_string(current_token.column) + ".");
    }

    // Handle member access (e.g., struct_instance.member)
    while (peek().type == Token::DOT) {
        consume(); // Consume '.'
        const Token& member_name_token = consume();
        if (member_name_token.type != Token::IDENTIFIER) {
            throw std::runtime_error("Expected identifier after '.' for member access.");
        }
        node = std::make_unique<MemberAccessNode>(std::move(node), member_name_token.value, member_name_token.line, member_name_token.column);
    }

    return node;
}

std::unique_ptr<ASTNode> Parser::parseUnaryExpression() {
    if (peek().type == Token::STAR || peek().type == Token::ADDRESSOF) {
        const Token& op_token = consume();
        auto operand = parseUnaryExpression();
        return std::make_unique<UnaryOpExpressionNode>(op_token.type, std::move(operand), op_token.line, op_token.column);
    }
    return parseFactor();
}

std::unique_ptr<ASTNode> Parser::parseTerm() {
    auto left_expr = parseUnaryExpression();

    while (peek().type == Token::STAR || peek().type == Token::SLASH) {
        const Token& op_token = consume();
        auto right_expr = parseUnaryExpression();
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

std::unique_ptr<StructDefinitionNode> Parser::parseStructDefinition() {
    consume(); // Consume 'struct' keyword
    std::string struct_name = consume().value; // Consume struct name
    expect(Token::LBRACE, "Expected '{' after struct name.");

    auto struct_node = std::make_unique<StructDefinitionNode>(struct_name);
    int current_offset = 0;

    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        auto member_type = parseType();
        std::string member_name = consume().value;
        expect(Token::SEMICOLON, "Expected ';' after struct member declaration.");

        int member_size = 0;
        if (auto primitive_type = dynamic_cast<PrimitiveTypeNode*>(member_type.get())) {
            switch (primitive_type->primitive_type) {
                case Token::KEYWORD_INT: member_size = 4;
                    break;
                case Token::KEYWORD_CHAR: member_size = 1;
                    break;
                case Token::KEYWORD_BOOL: member_size = 1;
                    break;
                default: member_size = 0; // Should not happen
            }
        } else if (auto pointer_type = dynamic_cast<PointerTypeNode*>(member_type.get())) {
            member_size = 8; // Size of a pointer
        } else if (auto array_type = dynamic_cast<ArrayTypeNode*>(member_type.get())) {
            // This is a simplification. A proper implementation would need to know the size of the base type.
            member_size = 8; // Treat array as a pointer for now
        } else if (auto struct_type = dynamic_cast<StructTypeNode*>(member_type.get())) {
            Symbol* struct_symbol = symbol_table.lookup(struct_type->struct_name);
            if (struct_symbol) {
                member_size = struct_symbol->size;
            }
        }

        struct_node->members.push_back({std::move(member_type), member_name, current_offset});
        current_offset += member_size;
    }

    struct_node->size = current_offset;
    expect(Token::RBRACE, "Expected '}' after struct definition.");
    symbol_table.addSymbol(Symbol(Symbol::SymbolType::STRUCT_DEFINITION, struct_name, std::move(struct_node)));
    return nullptr; // The struct definition is now owned by the symbol table
}

std::unique_ptr<ASTNode> Parser::parseStatement() {
    switch (peek().type) {
        case Token::KEYWORD_RETURN:
            return parseReturnStatement();
        case Token::KEYWORD_INT:
        case Token::KEYWORD_STRING:
        case Token::KEYWORD_BOOL:
        case Token::KEYWORD_CHAR: {
            auto decl_node = parseVariableDeclaration();
            expect(Token::SEMICOLON, "Expected ';' after variable declaration.");
            return decl_node;
        }
        case Token::IDENTIFIER: {
            // It could be a variable declaration with a struct type
            if (symbol_table.isStructDefined(peek().value)) {
                auto decl_node = parseVariableDeclaration();
                expect(Token::SEMICOLON, "Expected ';' after variable declaration.");
                return decl_node;
            }
            if (peek(1).type == Token::LPAREN) {
                auto functionCall = parseFunctionCall();
                expect(Token::SEMICOLON, "Expected ';' after function call statement.");
                return functionCall;
            } else {
                auto assign_node = parseVariableAssignment();
                expect(Token::SEMICOLON, "Expected ';' after variable assignment.");
                return assign_node;
            }
        }
        case Token::KEYWORD_PRINT:
            return parsePrintStatement();
        case Token::KEYWORD_IF:
            return parseIfStatement();
        case Token::KEYWORD_WHILE:
            return parseWhileStatement();
        case Token::KEYWORD_FOR:
            return parseForStatement();
        default:
            throw std::runtime_error("Parser Error: Unexpected token in statement: '" +
                                     peek().value + "' at line " + std::to_string(peek().line) +
                                     ", column " + std::to_string(peek().column) + ".");
    }
}

std::vector<std::unique_ptr<ParameterNode>> Parser::parseParameters() {
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    expect(Token::LPAREN, "Expected '(' after function name.");

    if (peek().type != Token::RPAREN) {
        do {
            auto param = std::make_unique<ParameterNode>();
            param->type = parseType();

            const Token& name_token = consume();
            if (name_token.type != Token::IDENTIFIER) {
                throw std::runtime_error("Expected identifier for parameter name.");
            }
            param->name = name_token.value;
            parameters.push_back(std::move(param));

        } while (peek().type == Token::COMMA && consume().type == Token::COMMA);
    }

    expect(Token::RPAREN, "Expected ')' after function parameters.");
    return parameters;
}

std::unique_ptr<FunctionDefinitionNode> Parser::parseFunctionDefinition() {
    const Token& return_type_token = peek();
    if (return_type_token.type != Token::KEYWORD_INT && return_type_token.type != Token::KEYWORD_VOID &&
        return_type_token.type != Token::KEYWORD_STRING && return_type_token.type != Token::KEYWORD_BOOL &&
        return_type_token.type != Token::KEYWORD_CHAR && !symbol_table.isStructDefined(return_type_token.value)) { // Allow struct return types
        throw std::runtime_error("Expected function return type (e.g., 'int', 'void', 'string', 'bool', 'char', or a defined struct).");
    }
    consume();

    const Token& function_name_token = peek();
    expect(Token::IDENTIFIER, "Expected function name.");

    auto func_def_node = std::make_unique<FunctionDefinitionNode>(
        return_type_token.type, function_name_token.value,
        return_type_token.line, function_name_token.column // Corrected column for function name
    );

    // Enter a new scope for the function body
    symbol_table.enterScope();

    func_def_node->parameters = parseParameters();

    // Add parameters to the symbol table
    for (const auto& param : func_def_node->parameters) {
        symbol_table.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, param->name, param->type->clone(), 0, 0)); // Placeholder offset/size
    }

    expect(Token::LBRACE, "Expected '{' to begin function body.");

    while (peek().type != Token::RBRACE && peek().type != Token::END_OF_FILE) {
        func_def_node->body_statements.push_back(parseStatement());
    }

    expect(Token::RBRACE, "Expected '}' to end function body.");

    // Exit the scope for the function body
    symbol_table.exitScope();

    return func_def_node;
}

std::unique_ptr<ProgramNode> Parser::parse() {
    auto program_node = std::make_unique<ProgramNode>();
    while (peek().type != Token::END_OF_FILE) {
        if (peek().type == Token::KEYWORD_INT ||
            peek().type == Token::KEYWORD_STRING ||
            peek().type == Token::KEYWORD_VOID ||
            peek().type == Token::KEYWORD_BOOL ||
            peek().type == Token::KEYWORD_CHAR) {
            // Could be a function definition or a variable declaration
            // For simplicity, assume function definition if followed by identifier and LPAREN
            // This needs more robust handling for actual type parsing
            size_t temp_index = current_token_index;
            Token type_token = consume(); // Consume type keyword
            if (peek().type == Token::IDENTIFIER) {
                consume(); // Consume identifier (function name)
                if (peek().type == Token::LPAREN) {
                    current_token_index = temp_index; // Rewind
                    program_node->functions.push_back(parseFunctionDefinition());
                } else {
                    current_token_index = temp_index; // Rewind
                    program_node->statements.push_back(parseVariableDeclaration());
                }
            } else {
                current_token_index = temp_index; // Rewind if not identifier
                program_node->statements.push_back(parseStatement());
            }
        } else if (peek().type == Token::KEYWORD_STRUCT) {
            parseStructDefinition();
            if (peek().type == Token::SEMICOLON) {
                consume(); // Consume optional semicolon after struct definition
            }
        } else {
            program_node->statements.push_back(parseStatement());
        }
    }
    return program_node;
}
