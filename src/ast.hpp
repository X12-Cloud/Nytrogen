#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory> // Smart pointers
#include "lexer.hpp"

// Base node class for all AST elements
struct ASTNode {
    enum class NodeType {
        PROGRAM = 0,
        INTEGER_LITERAL_EXPRESSION = 1,
        RETURN_STATEMENT = 2,
        VARIABLE_DECLARATION = 3,
        VARIABLE_ASSIGNMENT = 4,
        VARIABLE_REFERENCE = 5,
        FUNCTION_DEFINITION = 6,
	BINARY_OPERATION_EXPRESSION = 7,
	PRINT_STATEMENT = 8,
	STRING_LITERAL_EXPRESSION = 9,
    };

    NodeType node_type;
    int line;    // Source line position
    int column;  // Source column position

    ASTNode(NodeType type, int line = -1, int column = -1)
        : node_type(type), line(line), column(column) {}

    virtual ~ASTNode() = default;
};

// Node representing integer literals (e.g., 42)
struct IntegerLiteralExpressionNode : public ASTNode {
    int value;

    IntegerLiteralExpressionNode(int val, int line = -1, int column = -1)
        : ASTNode(NodeType::INTEGER_LITERAL_EXPRESSION, line, column), value(val) {}
};

// Node representing string literals (e.g., "Hello World")
struct StringLiteralExpressionNode : public ASTNode {
    std::string value;

    StringLiteralExpressionNode(std::string val, int line = -1, int column = -1)
        : ASTNode(NodeType::STRING_LITERAL_EXPRESSION, line, column), value(std::move(val)) {}
};

// Node for return statements (e.g., return x;)
struct ReturnStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;

    ReturnStatementNode(std::unique_ptr<ASTNode> expr, int line = -1, int column = -1)
        : ASTNode(NodeType::RETURN_STATEMENT, line, column), expression(std::move(expr)) {}
};

// Node for variable declarations (e.g., var x;)
struct VariableDeclarationNode : public ASTNode {
    std::string name;

    VariableDeclarationNode(std::string var_name, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_DECLARATION, line, column), name(std::move(var_name)) {}
};

// Node for variable assignments (e.g., x = 5;)
struct VariableAssignmentNode : public ASTNode {
    std::string name;
    std::unique_ptr<ASTNode> expression;

    VariableAssignmentNode(std::string var_name, std::unique_ptr<ASTNode> expr, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_ASSIGNMENT, line, column),
          name(std::move(var_name)),
          expression(std::move(expr)) {}
};

// Node for variable references in expressions (e.g., x in x + 1)
struct VariableReferenceNode : public ASTNode {
    std::string name;

    VariableReferenceNode(std::string var_name, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_REFERENCE, line, column), name(std::move(var_name)) {}
};

// Node for function definitions (e.g., int main()
struct FunctionDefinitionNode : public ASTNode {
    Token::Type return_type_token;
    std::string name;

    std::vector<std::unique_ptr<ASTNode>> body_statements;

    FunctionDefinitionNode(Token::Type ret_type, const std::string& func_name, int line = -1, int column = -1)
        : ASTNode(NodeType::FUNCTION_DEFINITION, line, column),
          return_type_token(ret_type),
	            name(func_name) {}
};

// Node for Arthemetic expression
struct BinaryOperationExpressionNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    Token::Type op_type;
    std::unique_ptr<ASTNode> right;

    BinaryOperationExpressionNode(std::unique_ptr<ASTNode> left_expr, Token::Type op, std::unique_ptr<ASTNode> right_expr, int line = -1, int column = -1)
        : ASTNode(NodeType::BINARY_OPERATION_EXPRESSION, line, column),
          left(std::move(left_expr)),
          op_type(op),
          right(std::move(right_expr)) {}
};

// Node for print statements (e.g., 'print x;')
struct PrintStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> expression; // The expression whose value will be printed

    PrintStatementNode(std::unique_ptr<ASTNode> expr, int line = -1, int column = -1)
        : ASTNode(NodeType::PRINT_STATEMENT, line, column),
          expression(std::move(expr)) {}
};

// Root node that contains all program statements
struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    std::vector<std::unique_ptr<FunctionDefinitionNode>> functions;

    ProgramNode(int line = -1, int column = -1)
        : ASTNode(NodeType::PROGRAM, line, column) {}
};

#endif // AST_HPP
