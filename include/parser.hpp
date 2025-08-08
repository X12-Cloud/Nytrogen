#ifndef NYTROGEN_PARSER_HPP
#define NYTROGEN_PARSER_HPP

#include "lexer.hpp"
#include <vector>
#include <memory>
#include <string>
#include <map>
#include <stdexcept>
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

    std::unique_ptr<ASTNode> parseStatement(); // General statement parsing (e.g., return, var decl, assignment)
    std::unique_ptr<VariableDeclarationNode> parseVariableDeclaration();
    std::unique_ptr<VariableAssignmentNode> parseVariableAssignment();
    std::unique_ptr<VariableReferenceNode> parseVariableReference();
    std::unique_ptr<ReturnStatementNode> parseReturnStatement();
    std::unique_ptr<PrintStatementNode> parsePrintStatement();
    std::unique_ptr<IfStatementNode> parseIfStatement();
    std::unique_ptr<FunctionCallNode> parseFunctionCall();
    std::vector<std::unique_ptr<ParameterNode>> parseParameters();

    // Expression parsing methods (now hierarchical for precedence)
    std::unique_ptr<ASTNode> parseExpression(); 		// Handles + and - (lowest precedence)
    std::unique_ptr<ASTNode> parseComparisonExpression(); 	// Handles == and > or < and <= or >= 
    std::unique_ptr<ASTNode> parseTerm();       		// Handles * and / (medium precedence)
    std::unique_ptr<ASTNode> parseFactor();     		// Handles literals, variables, and parentheses (highest precedence)
    std::unique_ptr<ASTNode> parseAdditiveExpression();

    std::unique_ptr<IntegerLiteralExpressionNode> parseIntegerLiteralExpression(); // Specific helper for int literals
    std::unique_ptr<StringLiteralExpressionNode> parseStringLiteralExpression();
    std::unique_ptr<FunctionDefinitionNode> parseFunctionDefinition();
};

#endif // NYTROGEN_PARSER_HPP
