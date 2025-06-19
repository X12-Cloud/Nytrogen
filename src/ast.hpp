#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory> // Smart pointers

// Base node class for all AST elements
struct ASTNode {
    enum class NodeType {
        PROGRAM,
        INTEGER_LITERAL_EXPRESSION,
        RETURN_STATEMENT,
        VARIABLE_DECLARATION,
        VARIABLE_ASSIGNMENT,
        VARIABLE_REFERENCE = 5,
        ADDITION = 6
    };

    NodeType node_type;
    int line;    // Source line position
    int column;  // Source column position

    ASTNode(NodeType type, int line = -1, int column = -1)
        : node_type(type), line(line), column(column) {}

    virtual ~ASTNode() = default;
};

// Root node that contains all program statements
struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;

    ProgramNode(int line = -1, int column = -1)
        : ASTNode(NodeType::PROGRAM, line, column) {}
};

// Node representing integer literals (e.g., 42)
struct IntegerLiteralExpressionNode : public ASTNode {
    int value;

    IntegerLiteralExpressionNode(int val, int line = -1, int column = -1)
        : ASTNode(NodeType::INTEGER_LITERAL_EXPRESSION, line, column), value(val) {}
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

#endif // AST_HPP
