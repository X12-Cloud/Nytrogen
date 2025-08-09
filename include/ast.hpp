#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory> // Smart pointers
#include "lexer.hpp"

// Forward declarations for type nodes
struct TypeNode;
struct PointerTypeNode;
struct ArrayTypeNode;

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
        IF_STATEMENT = 10,
        ELSE_STATEMENT = 11,
        FUNCTION_CALL = 12,
        WHILE_STATEMENT = 13,
        BOOLEAN_LITERAL_EXPRESSION = 14,
        CHARACTER_LITERAL_EXPRESSION = 15,
        FOR_STATEMENT = 16,
        UNARY_OP_EXPRESSION = 17,
        ARRAY_ACCESS_EXPRESSION = 18,
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

// Node representing boolean literals (e.g., true)
struct BooleanLiteralExpressionNode : public ASTNode {
    bool value;

    BooleanLiteralExpressionNode(int val, int line = -1, int column = -1)
        : ASTNode(NodeType::BOOLEAN_LITERAL_EXPRESSION, line, column), value(val) {}
};

// Node representing character literals (e.g., 'x')
struct CharacterLiteralExpressionNode : public ASTNode {
    char value;

    CharacterLiteralExpressionNode(int val, int line = -1, int column = -1)
        : ASTNode(NodeType::CHARACTER_LITERAL_EXPRESSION, line, column), value(val) {}
};

// Node for return statements (e.g., return x;)
struct ReturnStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;

    ReturnStatementNode(std::unique_ptr<ASTNode> expr, int line = -1, int column = -1)
	: ASTNode(NodeType::RETURN_STATEMENT, line, column), expression(std::move(expr)) {}
};

// Base class for type representations
struct TypeNode {
    enum class TypeCategory {
        PRIMITIVE,
        POINTER,
        ARRAY
    };
    TypeCategory category;
    TypeNode(TypeCategory cat) : category(cat) {}
    virtual ~TypeNode() = default;
};

struct PrimitiveTypeNode : public TypeNode {
    Token::Type primitive_type;
    PrimitiveTypeNode(Token::Type type) : TypeNode(TypeCategory::PRIMITIVE), primitive_type(type) {}
};

struct PointerTypeNode : public TypeNode {
    std::unique_ptr<TypeNode> base_type;
    PointerTypeNode(std::unique_ptr<TypeNode> base) : TypeNode(TypeCategory::POINTER), base_type(std::move(base)) {}
};

struct ArrayTypeNode : public TypeNode {
    std::unique_ptr<TypeNode> base_type;
    int size;
    ArrayTypeNode(std::unique_ptr<TypeNode> base, int sz) : TypeNode(TypeCategory::ARRAY), base_type(std::move(base)), size(sz) {}
};

// Node for variable declarations (e.g., int/string x;)
struct VariableDeclarationNode : public ASTNode {
    std::string name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ASTNode> initial_value;

    VariableDeclarationNode(std::string name, std::unique_ptr<TypeNode> type, std::unique_ptr<ASTNode> initial_val = nullptr, int line = -1, int column = -1)
	: ASTNode(NodeType::VARIABLE_DECLARATION, line, column), name(std::move(name)), type(std::move(type)), initial_value(std::move(initial_val)) {}
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

struct UnaryOpExpressionNode : public ASTNode {
    Token::Type op_type;
    std::unique_ptr<ASTNode> operand;

    UnaryOpExpressionNode(Token::Type op, std::unique_ptr<ASTNode> operand_node, int line = -1, int column = -1)
        : ASTNode(NodeType::UNARY_OP_EXPRESSION, line, column), op_type(op), operand(std::move(operand_node)) {}
};

struct ArrayAccessNode : public ASTNode {
    std::unique_ptr<ASTNode> array_expr;
    std::unique_ptr<ASTNode> index_expr;

    ArrayAccessNode(std::unique_ptr<ASTNode> array, std::unique_ptr<ASTNode> index, int line = -1, int column = -1)
        : ASTNode(NodeType::ARRAY_ACCESS_EXPRESSION, line, column), array_expr(std::move(array)), index_expr(std::move(index)) {}
};

struct ParameterNode {
    std::unique_ptr<TypeNode> type;
    std::string name;
};


// Node for function definitions (e.g., int main() {})
struct FunctionDefinitionNode : public ASTNode {
    Token::Type return_type_token;
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    std::vector<std::unique_ptr<ASTNode>> body_statements;

    FunctionDefinitionNode(Token::Type ret_type, const std::string& func_name, int line = -1, int column = -1)
        : ASTNode(NodeType::FUNCTION_DEFINITION, line, column),
          return_type_token(ret_type),
	            name(func_name) {}
};

// Node for function calls
struct FunctionCallNode : public ASTNode {
    std::string function_name;
    std::vector<std::unique_ptr<ASTNode>> arguments;

    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args, int line = -1, int column = -1)
        : ASTNode(NodeType::FUNCTION_CALL, line, column),
          function_name(std::move(name)),
          arguments(std::move(args)) {}
};

// Node for while statements
struct WhileStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

    WhileStatementNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> body_stmts, int line = -1, int column = -1)
        : ASTNode(NodeType::WHILE_STATEMENT, line, column),
          condition(std::move(cond)),
          body(std::move(body_stmts)) {}
};

// Node for for statements
struct ForStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> initializer;
    std::unique_ptr<ASTNode> condition;
    std::unique_ptr<ASTNode> increment;
    std::vector<std::unique_ptr<ASTNode>> body;

    ForStatementNode(std::unique_ptr<ASTNode> init, std::unique_ptr<ASTNode> cond, std::unique_ptr<ASTNode> incr, std::vector<std::unique_ptr<ASTNode>> body_stmts,
                     int line = -1, int column = -1)
        : ASTNode(NodeType::FOR_STATEMENT, line, column),
          initializer(std::move(init)),
          condition(std::move(cond)),
          increment(std::move(incr)),
          body(std::move(body_stmts)) {}
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

struct IfStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> true_block;
    std::vector<std::unique_ptr<ASTNode>> false_block;

    IfStatementNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> t_block,
                    std::vector<std::unique_ptr<ASTNode>> f_block = {},
                    int line = -1, int column = -1)
        : ASTNode(NodeType::IF_STATEMENT, line, column),
          condition(std::move(cond)),
          true_block(std::move(t_block)),
          false_block(std::move(f_block)) {}
};

// Root node that contains all program statements
struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    std::vector<std::unique_ptr<FunctionDefinitionNode>> functions;

    ProgramNode(int line = -1, int column = -1)
        : ASTNode(NodeType::PROGRAM, line, column) {}
};

#endif // AST_HPP
