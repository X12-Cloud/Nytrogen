#ifndef AST_HPP
#define AST_HPP

#include "utils.hpp"
#include <string>
#include <vector>
#include <memory>
#include <iostream>
#include <fstream>
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
	    CONSTANT_LITERAL_EXPRESSION = 22,
	    ENUM_STATEMENT = 23,
	    CONSTANT_DECLARATION = 24,
	    FLOAT_LITERAL_EXPRESSION = 25,
	    DOUBLE_LITERAL_EXPRESSION = 26,
	    SWITCH_STATEMENT = 27,
    };

    NodeType node_type;

    // resolved_type
    std::shared_ptr<TypeNode> resolved_type;	

    int line;    // Source line position
    int column;  // Source column position

    ASTNode(NodeType type, int line = -1, int column = -1)
        : node_type(type), line(line), column(column) {}

    virtual ~ASTNode() = default;

    virtual std::string type_name() const {
        return "ASTNode"; 
    }

    virtual std::vector<ASTNode*> get_children() const {
        return {}; // Base node has no children
    }

    virtual std::string get_value() const {
        return ""; 
    }

    void dump_to_stream(std::ostream& out, int indent) { // TODO: make it output to .json
        std::string space(indent * 2, ' ');
        out << space << " " << this->type_name();

        std::string val = this->get_value();
        if (!val.empty()) out << " (" << val << ")";
        out << std::endl;

        for (auto* child : get_children()) {
            if (child) child->dump_to_stream(out, indent + 1);
        }
    }

    void dump() {
        std::ofstream ast_file("ast.txt", std::ios::trunc);
        if (ast_file.is_open()) {
            this->dump_to_stream(ast_file, 0);
            ast_file.close();
        }
    }
};

// Node representing all literals
struct LiteralExpressionNode : public ASTNode {
    LiteralExpressionNode(NodeType type, int line, int column) 
        : ASTNode(type, line, column) {}

    virtual std::string getValueAsString() const = 0;
};

// Node representing integer literals (e.g., 42)
struct IntegerLiteralExpressionNode : public LiteralExpressionNode {
    int value;

    std::string type_name() const override { return "INT_LITERAL:"; }
    std::string get_value() const override { return getValueAsString(); }

    IntegerLiteralExpressionNode(int val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::INTEGER_LITERAL_EXPRESSION, line, column), value(val) {}
    std::string getValueAsString() const override { return std::to_string(value); }
};

// Node representing string literals (e.g., "Hello World")
struct StringLiteralExpressionNode : public LiteralExpressionNode {
    std::string value;

    std::string type_name() const override { return "STR_LITERAL:"; }
    std::string get_value() const override { return getValueAsString(); }

    StringLiteralExpressionNode(std::string val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::STRING_LITERAL_EXPRESSION, line, column), value(std::move(val)) {}
    std::string getValueAsString() const override { return value; }
};

// Node representing boolean literals (e.g., true)
struct BooleanLiteralExpressionNode : public LiteralExpressionNode {
    bool value;

    std::string type_name() const override { return "BOOL_LITERAL:"; }
    std::string get_value() const override { return getValueAsString(); }

    BooleanLiteralExpressionNode(int val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::BOOLEAN_LITERAL_EXPRESSION, line, column), value(val) {}
    std::string getValueAsString() const override { return std::to_string(value); }
};

// Node representing character literals (e.g., 'x')
struct CharacterLiteralExpressionNode : public LiteralExpressionNode {
    char value;

    std::string type_name() const override { return "CHAR_LITERAL:"; }
    std::string get_value() const override { return getValueAsString(); }

    CharacterLiteralExpressionNode(int val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::CHARACTER_LITERAL_EXPRESSION, line, column), value(val) {}
    std::string getValueAsString() const override { return std::to_string(value); }
};

// Node representing float literals
struct FloatLiteralExpressionNode : public LiteralExpressionNode {
    float value;
    std::string label;

    std::string type_name() const override { return "FLOAT_LITERAL:"; }
    std::string get_value() const override { return getValueAsString(); }

    FloatLiteralExpressionNode(float val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::FLOAT_LITERAL_EXPRESSION, line, column), value(val), label("") {}
    std::string getValueAsString() const override { return std::to_string(value); }
};

// Node representing double literals
struct DoubleLiteralExpressionNode : public LiteralExpressionNode {
    double value;
    std::string label;

    std::string type_name() const override { return "DOUBLE_LITERAL:"; }
    std::string get_value() const override { return getValueAsString(); }

    DoubleLiteralExpressionNode(double val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::DOUBLE_LITERAL_EXPRESSION, line, column), value(val), label("") {}
    std::string getValueAsString() const override { return std::to_string(value); }
};

// Node for return statements (e.g., return x;)
struct ReturnStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;

    std::string type_name() const override { return "RETURN_STMT:"; }
    std::vector<ASTNode*> get_children() const override {
        return { expression.get() };
    }

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

struct AutoTypeNode : public TypeNode {
    AutoTypeNode() : TypeNode(TypeCategory::PRIMITIVE) {} // Treat as primitive for simplicity, actual type deduced later
    std::unique_ptr<TypeNode> clone() const override {
        return std::make_unique<AutoTypeNode>();
    }
};

struct StructMember {
    enum class Visibility {
        PUBLIC,
        PRIVATE
    };

    std::unique_ptr<TypeNode> type;
    std::string name;
    int offset;
    Visibility visibility = Visibility::PUBLIC; // Default to public
};

struct StructDefinitionNode : public ASTNode {
    std::string name;
    std::vector<StructMember> members;
    int size;

    std::string type_name() const override {
        std::string info = "STRUCT_DEF: " + name + " { ";
        for (const auto& m : members) {
            info += m.name + " "; // Just the names for simplicity
        }
        info += "}";
        return info;
    }
    std::vector<ASTNode*> get_children() const override { return {}; }

    StructDefinitionNode(std::string struct_name, int line = -1, int column = -1)
        : ASTNode(NodeType::STRUCT_DEFINITION, line, column), name(std::move(struct_name)), size(0) {}

    std::shared_ptr<StructDefinitionNode> clone() const {
        auto new_node = std::make_shared<StructDefinitionNode>(name, line, column);
        new_node->size = size;
        for (const auto& m : members) {
            StructMember cloned_m;
            cloned_m.name = m.name;
            cloned_m.offset = m.offset;
            cloned_m.visibility = m.visibility;
            cloned_m.type = m.type->clone();
            new_node->members.push_back(std::move(cloned_m));
        }
        return new_node;
    }
};

struct MemberAccessNode : public ASTNode {
    std::unique_ptr<ASTNode> struct_expr; // The expression representing the struct instance
    std::string member_name;
    Symbol* resolved_symbol;

    std::string type_name() const override { return "MEMBER_ACCESS: " + member_name; }

    MemberAccessNode(std::unique_ptr<ASTNode> expr, std::string member, int line = -1, int column = -1)
        : ASTNode(NodeType::MEMBER_ACCESS_EXPRESSION, line, column),
          struct_expr(std::move(expr)),
          member_name(std::move(member)),
          resolved_symbol(nullptr) {}
};

struct ExpressionNode; // Forward declaration

struct Declaration {
    std::string name;
    std::unique_ptr<ASTNode> initial_value; 
    Symbol* resolved_symbol = nullptr;
};

// Node for variable declarations (e.g., int/string x;)
struct VariableDeclarationNode : public ASTNode {
    std::unique_ptr<TypeNode> type;
    std::vector<Declaration> declarations;

    std::string type_name() const override { return "VAR_DECL"; }
    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        for (auto& decl : declarations) {
            if (decl.initial_value) refs.push_back(decl.initial_value.get());
        }
        return refs;
    }

    VariableDeclarationNode(std::unique_ptr<TypeNode> type, std::vector<Declaration> decls)
        : ASTNode(NodeType::VARIABLE_DECLARATION), 
          type(std::move(type)), 
          declarations(std::move(decls)) {}
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
    Symbol* resolved_symbol;
    int resolved_offset;
    std::unique_ptr<TypeNode> resolved_type;

    std::string type_name() const override { return "VAR_REF:"; }
    std::string get_value() const override { return name; }

    VariableReferenceNode(std::string var_name, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_REFERENCE, line, column), name(std::move(var_name)), resolved_symbol(nullptr), resolved_offset(0) {}
};

// Node for unary operations.
struct UnaryOpExpressionNode : public ASTNode {
    Token::Type op_type;
    std::unique_ptr<ASTNode> operand;
    Symbol* resolved_symbol;
    std::unique_ptr<TypeNode> resolved_type;

    UnaryOpExpressionNode(Token::Type op, std::unique_ptr<ASTNode> operand_node, int line = -1, int column = -1)
        : ASTNode(NodeType::UNARY_OP_EXPRESSION, line, column), op_type(op), operand(std::move(operand_node)), resolved_symbol(nullptr) {}
};

struct ArrayAccessNode : public ASTNode {
    std::unique_ptr<ASTNode> array_expr;
    std::unique_ptr<ASTNode> index_expr;
    Symbol* resolved_symbol;

    std::string type_name() const override { return "ARRAY_ACCESS"; }
    std::vector<ASTNode*> get_children() const override {
        return { array_expr.get(), index_expr.get() };
    } 

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

    std::string type_name() const override { return "FUNCTION_DEF: " + name; }
    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        for (auto& stmt : body_statements) refs.push_back(stmt.get());
        return refs;
    }

    FunctionDefinitionNode(std::unique_ptr<TypeNode> ret_type, const std::string& func_name, int line = -1, int column = -1)
        : ASTNode(NodeType::FUNCTION_DEFINITION, line, column),
          return_type(std::move(ret_type)),
	            name(func_name), is_extern(false) {}
    bool is_extern;
};

// Node for function calls
struct FunctionCallNode : public ASTNode {
    std::string function_name;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    Symbol* resolved_symbol;

    std::string type_name() const override { return "FUNC_CALL: " + function_name; }
    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        for (auto& arg : arguments) refs.push_back(arg.get());
        return refs;
    }

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
    std::unique_ptr<TypeNode> resolved_type;

    std::string type_name() const override { return "BINARY_OP: "; }
    std::vector<ASTNode*> get_children() const override {
        return { left.get(), right.get() };
    }

    BinaryOperationExpressionNode(std::unique_ptr<ASTNode> left_expr, Token::Type op, std::unique_ptr<ASTNode> right_expr, int line = -1, int column = -1)
        : ASTNode(NodeType::BINARY_OPERATION_EXPRESSION, line, column),
          left(std::move(left_expr)),
          op_type(op),
          right(std::move(right_expr)) {}
};

// Node for print statements (e.g., 'print x, "hello";')
struct PrintStatementNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> expressions;

    std::string type_name() const override { return "PRINT_STMT"; }
    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        for (auto& expr : expressions) refs.push_back(expr.get());
        return refs;
    }

    PrintStatementNode(std::vector<std::unique_ptr<ASTNode>> exprs, int line = -1, int column = -1)
        : ASTNode(NodeType::PRINT_STATEMENT, line, column),
          expressions(std::move(exprs)) {}
};

// Node for if statements.
struct IfStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> true_block;
    std::vector<std::unique_ptr<ASTNode>> false_block;

    std::string type_name() const override { return "IF_STATEMENT"; }
    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        if (condition) refs.push_back(condition.get());
        for (auto& stmt : true_block) refs.push_back(stmt.get());
        for (auto& stmt : false_block) refs.push_back(stmt.get());
        return refs;
    }

    IfStatementNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> t_block,
                    std::vector<std::unique_ptr<ASTNode>> f_block = {},
                    int line = -1, int column = -1)
        : ASTNode(NodeType::IF_STATEMENT, line, column),
          condition(std::move(cond)),
          true_block(std::move(t_block)),
          false_block(std::move(f_block)) {}
};

// Node for switch statements.
struct CaseNode {
    std::unique_ptr<ASTNode> constant_expr;
    std::vector<std::unique_ptr<ASTNode>> body;
    bool is_default = false;
};

struct SwitchStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<CaseNode> cases;

    bool use_jump_table = false;
    long long min_case = 0;
    long long max_case = 0;

    std::string type_name() const override { return "SWITCH_STATEMENT"; }
    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        if (condition) refs.push_back(condition.get());
        for (auto& c : cases) {
            if (c.constant_expr) refs.push_back(c.constant_expr.get());
            for (auto& stmt : c.body) {
                refs.push_back(stmt.get());
            }
        }
        return refs;
    }

    SwitchStatementNode(int line = -1, int column = -1) 
        : ASTNode(NodeType::SWITCH_STATEMENT, line, column) {}
};

// Root node that contains all program statements
struct ProgramNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> statements;
    std::vector<std::unique_ptr<FunctionDefinitionNode>> functions;
    std::vector<std::unique_ptr<StructDefinitionNode>> structs;

    std::vector<ASTNode*> get_children() const override {
        std::vector<ASTNode*> refs;
        for (auto& stmt : statements) refs.push_back(stmt.get());
        for (auto& func : functions) refs.push_back(func.get());
        for (auto& str : structs) refs.push_back(str.get());
        return refs;
    }

    std::string type_name() const override { return "PROGRAM_ROOT"; }

    ProgramNode(int line = -1, int column = -1)
        : ASTNode(NodeType::PROGRAM, line, column) {}
};


// Node for inline assembly blocks
struct AsmStatementNode : public ASTNode {
    std::vector<std::string> lines;
    AsmStatementNode(std::vector<std::string> asm_lines, int line = -1, int column = -1)
        : ASTNode(NodeType::ASM_STATEMENT, line, column), lines(std::move(asm_lines)) {}
};

// Node for constant declarations (e.g., const int x = 5;)
struct ConstantDeclarationNode : public ASTNode {
    std::string name;
    std::unique_ptr<TypeNode> type;
    std::unique_ptr<ASTNode> initial_value;
    Symbol* resolved_symbol;

    std::string type_name() const override { return "CONST_DECL: " + name; }
    std::vector<ASTNode*> get_children() const override {
        return { initial_value.get() };
    }

    ConstantDeclarationNode(std::string name, std::unique_ptr<TypeNode> type, std::unique_ptr<ASTNode> initial_val, int line = -1, int column = -1)
        : ASTNode(NodeType::CONSTANT_DECLARATION, line, column), name(std::move(name)), type(std::move(type)), initial_value(std::move(initial_val)), resolved_symbol(nullptr) {}
};

struct EnumMemberNode {
    std::string name;
    std::unique_ptr<ASTNode> value; // Can be nullptr for implicit values

    EnumMemberNode(std::string name, std::unique_ptr<ASTNode> value = nullptr)
        : name(std::move(name)), value(std::move(value)) {}
};

struct EnumStatementNode : public ASTNode {
    std::string name;
    std::vector<std::unique_ptr<EnumMemberNode>> members;

    std::string type_name() const override { return "ENUM: " + name; }
    std::vector<ASTNode*> get_children() const override { return {}; }

    EnumStatementNode(std::string name, std::vector<std::unique_ptr<EnumMemberNode>> members, int line = -1, int column = -1)
        : ASTNode(NodeType::ENUM_STATEMENT, line, column), name(std::move(name)), members(std::move(members)) {}
};

#endif // AST_HPP
