#ifndef AST_HPP
#define AST_HPP

#include <string>
#include <vector>
#include <memory> // Smart pointers
#include "lexer.hpp"

struct Symbol; // Forward declaration for Symbol

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
        STRUCT_DEFINITION = 19,
        MEMBER_ACCESS_EXPRESSION = 20,
	ASM_STATEMENT = 21,
    };

    NodeType node_type;

    // resolved_type
    std::shared_ptr<TypeNode> resolved_type;	

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
        ARRAY,
        STRUCT
    };
    TypeCategory category;
    TypeNode(TypeCategory cat) : category(cat) {}
    virtual ~TypeNode() = default;
    virtual std::unique_ptr<TypeNode> clone() const = 0;
};

struct PrimitiveTypeNode : public TypeNode {
    Token::Type primitive_type;
    PrimitiveTypeNode(Token::Type type) : TypeNode(TypeCategory::PRIMITIVE), primitive_type(type) {}
    std::unique_ptr<TypeNode> clone() const override {
        return std::make_unique<PrimitiveTypeNode>(primitive_type);
    }
};

struct PointerTypeNode : public TypeNode {
    std::unique_ptr<TypeNode> base_type;
    PointerTypeNode(std::unique_ptr<TypeNode> base) : TypeNode(TypeCategory::POINTER), base_type(std::move(base)) {}
    std::unique_ptr<TypeNode> clone() const override {
        return std::make_unique<PointerTypeNode>(base_type->clone());
    }
};

struct ArrayTypeNode : public TypeNode {
    std::unique_ptr<TypeNode> base_type;
    int size;
    ArrayTypeNode(std::unique_ptr<TypeNode> base, int sz) : TypeNode(TypeCategory::ARRAY), base_type(std::move(base)), size(sz) {}
    std::unique_ptr<TypeNode> clone() const override {
        return std::make_unique<ArrayTypeNode>(base_type->clone(), size);
    }
};

struct StructTypeNode : public TypeNode {
    std::string struct_name;
    StructTypeNode(std::string name) : TypeNode(TypeCategory::STRUCT), struct_name(std::move(name)) {}
    std::unique_ptr<TypeNode> clone() const override {
        return std::make_unique<StructTypeNode>(struct_name);
    }
};

struct StructMember {
    std::unique_ptr<TypeNode> type;
    std::string name;
    int offset; // Add offset for each member
};

struct StructDefinitionNode : public ASTNode {
    std::string name;
    std::vector<StructMember> members;
    int size; // Add size for the struct

    StructDefinitionNode(std::string struct_name, int line = -1, int column = -1)
        : ASTNode(NodeType::STRUCT_DEFINITION, line, column), name(std::move(struct_name)), size(0) {}

    std::shared_ptr<StructDefinitionNode> clone() const {
        auto new_node = std::make_shared<StructDefinitionNode>(name, line, column);
        new_node->size = size;
        for (const auto& member : members) {
            new_node->members.push_back({member.type->clone(), member.name, member.offset});
        }
        return new_node;
    }
};

struct MemberAccessNode : public ASTNode {
    std::unique_ptr<ASTNode> struct_expr; // The expression representing the struct instance
    std::string member_name;
    Symbol* resolved_symbol; // Add resolved_symbol

    MemberAccessNode(std::unique_ptr<ASTNode> expr, std::string member, int line = -1, int column = -1)
        : ASTNode(NodeType::MEMBER_ACCESS_EXPRESSION, line, column),
          struct_expr(std::move(expr)),
          member_name(std::move(member)),
          resolved_symbol(nullptr) {}
};

// Node for variable declarations (e.g., int/string x;)
struct VariableDeclarationNode : public ASTNode {
    std::string name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ASTNode> initial_value;
    Symbol* resolved_symbol; // Add resolved_symbol

    VariableDeclarationNode(std::string name, std::unique_ptr<TypeNode> type, std::unique_ptr<ASTNode> initial_val = nullptr, int line = -1, int column = -1)
	: ASTNode(NodeType::VARIABLE_DECLARATION, line, column), name(std::move(name)), type(std::move(type)), initial_value(std::move(initial_val)), resolved_symbol(nullptr) {}
};


// Node for variable assignments (e.g., x = 5;)
struct VariableAssignmentNode : public ASTNode {
    std::unique_ptr<ASTNode> left;
    std::unique_ptr<ASTNode> right;

    VariableAssignmentNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_ASSIGNMENT, line, column),
          left(std::move(left)),
          right(std::move(right)) {}
};

// Node for variable references in expressions (e.g., x in x + 1)
struct VariableReferenceNode : public ASTNode {
    std::string name;
    Symbol* resolved_symbol; // Add resolved_symbol
    int resolved_offset; // Store the resolved offset
    std::unique_ptr<TypeNode> resolved_type; // Store the resolved type

    VariableReferenceNode(std::string var_name, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_REFERENCE, line, column), name(std::move(var_name)), resolved_symbol(nullptr), resolved_offset(0) {}
};

struct UnaryOpExpressionNode : public ASTNode {
    Token::Type op_type;
    std::unique_ptr<ASTNode> operand;
    Symbol* resolved_symbol; // Add resolved_symbol
    std::unique_ptr<TypeNode> resolved_type; // Add resolved_type

    UnaryOpExpressionNode(Token::Type op, std::unique_ptr<ASTNode> operand_node, int line = -1, int column = -1)
        : ASTNode(NodeType::UNARY_OP_EXPRESSION, line, column), op_type(op), operand(std::move(operand_node)), resolved_symbol(nullptr) {}
};

struct ArrayAccessNode : public ASTNode {
    std::unique_ptr<ASTNode> array_expr;
    std::unique_ptr<ASTNode> index_expr;
    Symbol* resolved_symbol; // Add resolved_symbol

    ArrayAccessNode(std::unique_ptr<ASTNode> array, std::unique_ptr<ASTNode> index, int line = -1, int column = -1)
        : ASTNode(NodeType::ARRAY_ACCESS_EXPRESSION, line, column), array_expr(std::move(array)), index_expr(std::move(index)), resolved_symbol(nullptr) {}
};

struct ParameterNode {
    std::unique_ptr<TypeNode> type;
    std::string name;
    int offset; // Add offset for parameter
};


// Node for function definitions (e.g., int main() {})
struct FunctionDefinitionNode : public ASTNode {
    std::unique_ptr<TypeNode> return_type;
    std::string name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    std::vector<std::unique_ptr<ASTNode>> body_statements;

    FunctionDefinitionNode(std::unique_ptr<TypeNode> ret_type, const std::string& func_name, int line = -1, int column = -1)
        : ASTNode(NodeType::FUNCTION_DEFINITION, line, column),
          return_type(std::move(ret_type)),
	            name(func_name) {}
};

// Node for function calls
struct FunctionCallNode : public ASTNode {
    std::string function_name;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    Symbol* resolved_symbol; // Add resolved_symbol

    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args, int line = -1, int column = -1)
        : ASTNode(NodeType::FUNCTION_CALL, line, column),
          function_name(std::move(name)),
          arguments(std::move(args)),
          resolved_symbol(nullptr) {}
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
    std::unique_ptr<TypeNode> resolved_type; // Add resolved_type

    BinaryOperationExpressionNode(std::unique_ptr<ASTNode> left_expr, Token::Type op, std::unique_ptr<ASTNode> right_expr, int line = -1, int column = -1)
        : ASTNode(NodeType::BINARY_OPERATION_EXPRESSION, line, column),
          left(std::move(left_expr)),
          op_type(op),
          right(std::move(right_expr)) {}
};

// Node for print statements (e.g., 'print x, "hello";')
struct PrintStatementNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> expressions; // The expressions whose values will be printed

    PrintStatementNode(std::vector<std::unique_ptr<ASTNode>> exprs, int line = -1, int column = -1)
        : ASTNode(NodeType::PRINT_STATEMENT, line, column),
          expressions(std::move(exprs)) {}
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
    std::vector<std::unique_ptr<StructDefinitionNode>> structs;

    ProgramNode(int line = -1, int column = -1)
        : ASTNode(NodeType::PROGRAM, line, column) {}
};


// Node for inline assembly blocks
struct AsmStatementNode : public ASTNode {
    std::vector<std::string> lines;
    AsmStatementNode(std::vector<std::string> asm_lines, int line = -1, int column = -1)
        : ASTNode(NodeType::ASM_STATEMENT, line, column), lines(std::move(asm_lines)) {}
};

#endif // AST_HPP
