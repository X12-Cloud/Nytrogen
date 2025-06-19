#ifndef NYTROGEN_PARSER_HPP
#define NYTROGEN_PARSER_HPP

#include "lexer.hpp"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include "ast.hpp"

// Parser class handles syntax analysis and AST construction
class Parser {
public:
    Parser(std::vector<Token> tokens);
    std::unique_ptr<ProgramNode> parse();

private:
    std::vector<Token> tokens;
    size_t current_token_index;
    std::map<std::string, int> declared_variables;

    // Token handling methods
    const Token& peek(size_t offset = 0) const;
    const Token& consume();
    void expect(Token::Type expected_type, const std::string& error_msg);

    // AST node parsing methods
    std::unique_ptr<ASTNode> parseStatement();
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclaration();
    std::unique_ptr<VariableAssignmentNode> parseVariableAssignment();
    std::unique_ptr<VariableReferenceNode> parseVariableReference();
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<ASTNode> parseExpression();
    std::unique_ptr<IntegerLiteralExpressionNode> parseIntegerLiteralExpression();
};

#endif // NYTROGEN_PARSER_HPP
