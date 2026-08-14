#ifndef AST_HPP
#define AST_HPP

#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <algorithm>
#include <sstream>
#include <iterator>

#include "lexer.hpp"
#include "utils/utils.hpp"

struct Symbol;  // Forward declaration for Symbol

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
        NAMESPACE_DEFINITION = 28,
        SCOPE_RESOLUTION = 29,
        COMPLEX_LITERAL_EXPRESSION = 30,
        QUBIT_DEFINITION = 31,
        GATE_APPLICATION_OPERATION_EXPRESSION = 32,
        FORMAT_EXPRESSION = 33,
    };

    NodeType node_type;

    // resolved_type
    std::shared_ptr<TypeNode> resolved_type;

    int line;    // Source line position
    int column;  // Source column position

    ASTNode(NodeType type, int line = -1, int column = -1)
        : node_type(type), line(line), column(column) {}

    virtual ~ASTNode() = default;

    [[nodiscard]] virtual auto type_name() const -> std::string {
        return "ASTNode";
    }

    [[nodiscard]] virtual auto get_children() const -> std::vector<ASTNode*> {
        return {};  // Base node has no children
    }

    [[nodiscard]] virtual auto get_value() const -> std::string {
        return "";
    }

    [[nodiscard]] virtual auto is_constant() const -> bool {
        return false;
    }

    void dump_to_stream(std::ostream& out,
                        int indent) const {  // TODO: make it output to .json and Graphvis
        std::string space(indent * 2, ' ');
        out << space << " " << this->type_name();

        std::string val = this->get_value();
        if (!val.empty()) {
            out << " (" << val << ")";
        }
        out << std::endl;

        for (auto* child : get_children()) {
            if (child != nullptr) {
                child->dump_to_stream(out, indent + 1);
            }
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
    LiteralExpressionNode(NodeType type, int line, int column) : ASTNode(type, line, column) {}

    [[nodiscard]] virtual auto getValueAsString() const -> std::string = 0;
};

// Node representing integer literals (e.g., 42)
struct IntegerLiteralExpressionNode : public LiteralExpressionNode {
    int value;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "INT_LITERAL:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }
    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    IntegerLiteralExpressionNode(int val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::INTEGER_LITERAL_EXPRESSION, line, column), value(val) {}
    [[nodiscard]] auto getValueAsString() const -> std::string override {
        return std::to_string(value);
    }
};

// Node representing string literals (e.g., "Hello World")
struct StringLiteralExpressionNode : public LiteralExpressionNode {
    std::string value;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "STR_LITERAL:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }
    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    StringLiteralExpressionNode(std::string val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::STRING_LITERAL_EXPRESSION, line, column),
          value(std::move(val)) {}
    [[nodiscard]] auto getValueAsString() const -> std::string override {
        return value;
    }
};

// Node representing boolean literals (e.g., true)
struct BooleanLiteralExpressionNode : public LiteralExpressionNode {
    bool value;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "BOOL_LITERAL:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }
    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    BooleanLiteralExpressionNode(int val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::BOOLEAN_LITERAL_EXPRESSION, line, column),
          value(val != 0) {}
    [[nodiscard]] auto getValueAsString() const -> std::string override {
        return value ? "true" : "false";
    }
};

// Node representing character literals (e.g., 'x')
struct CharacterLiteralExpressionNode : public LiteralExpressionNode {
    char value;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "CHAR_LITERAL:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }
    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    CharacterLiteralExpressionNode(int val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::CHARACTER_LITERAL_EXPRESSION, line, column), value(val) {}
    [[nodiscard]] auto getValueAsString() const -> std::string override {
        return std::to_string(value);
    }
};

// Node representing float literals
struct FloatLiteralExpressionNode : public LiteralExpressionNode {
    float value;
    std::string label;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "FLOAT_LITERAL:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }
    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    FloatLiteralExpressionNode(float val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::FLOAT_LITERAL_EXPRESSION, line, column), value(val) {}
    [[nodiscard]] auto getValueAsString() const -> std::string override {
        return std::to_string(value);
    }
};

// Node representing double literals
struct DoubleLiteralExpressionNode : public LiteralExpressionNode {
    double value;
    std::string label;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "DOUBLE_LITERAL:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }
    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    DoubleLiteralExpressionNode(double val, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::DOUBLE_LITERAL_EXPRESSION, line, column), value(val) {}
    [[nodiscard]] auto getValueAsString() const -> std::string override {
        return std::to_string(value);
    }
};

// Node representing complex literals (e.g., 1.0+2.0i)
struct ComplexLiteralExpressionNode : public LiteralExpressionNode {
    double real;
    double imaginary;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "COMPLEX_LITERAL:";
    }

    [[nodiscard]] auto get_value() const -> std::string override {
        return getValueAsString();
    }

    [[nodiscard]] auto is_constant() const -> bool override {
        return true;
    }

    ComplexLiteralExpressionNode(double re, double im, int line = -1, int column = -1)
        : LiteralExpressionNode(NodeType::COMPLEX_LITERAL_EXPRESSION, line, column), 
          real(re), imaginary(im) {}

    [[nodiscard]] auto getValueAsString() const -> std::string override {
        std::string sign = (imaginary >= 0) ? "+" : "";
        return std::to_string(real) + sign + std::to_string(imaginary) + "i";
    }
};

// Node for return statements (e.g., return x;)
struct ReturnStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> expression;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "RETURN_STMT:";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {expression.get()};
    }

    ReturnStatementNode(std::unique_ptr<ASTNode> expr, int line = -1, int column = -1)
        : ASTNode(NodeType::RETURN_STATEMENT, line, column), expression(std::move(expr)) {}
};

struct PrimitiveTypeNode;

// Node representing format expressions (e.g. (format "x = {}" x))
struct FormatExpressionNode : public ASTNode {
    std::unique_ptr<ASTNode> template_str;
    std::vector<std::unique_ptr<ASTNode>> expressions;
    int var_count;
    int placeholder_count;

    [[nodiscard]] auto type_name() const -> std::string override {
        std::ostringstream ss;
        std::string info = "FORMAT_EXPR: { ";
        for (size_t i = 0; i < expressions.size(); ++i) {
            info += expressions[i]->get_value() + " ";
        }
        info += "}";
        return info;
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {};
    }

    FormatExpressionNode(std::unique_ptr<ASTNode> str, 
                     std::vector<std::unique_ptr<ASTNode>> ids, 
                     int id_count, int line = -1, int column = -1)
    : ASTNode(NodeType::FORMAT_EXPRESSION, line, column), 
      template_str(std::move(str)),
      expressions(std::move(ids)),
      var_count(id_count) {}
};

// Base class for type representations
struct TypeNode {
    enum class TypeCategory { PRIMITIVE, POINTER, ARRAY, STRUCT };
    TypeCategory category;
    TypeNode(TypeCategory cat) : category(cat) {}
    virtual ~TypeNode() = default;
    [[nodiscard]] virtual auto typeName() const -> std::string = 0;
    [[nodiscard]] virtual auto clone() const -> std::unique_ptr<TypeNode> = 0;
};

struct PrimitiveTypeNode : public TypeNode {
    Token::Type primitive_type;
    PrimitiveTypeNode(Token::Type type) : TypeNode(TypeCategory::PRIMITIVE), primitive_type(type) {}

    [[nodiscard]] auto typeName() const -> std::string override {
        switch (primitive_type) {
            case Token::KEYWORD_INT:
                return "int";
            case Token::KEYWORD_FLOAT:
                return "float";
            case Token::KEYWORD_DOUBLE:
                return "double";
            case Token::KEYWORD_STRING:
                return "string";
            case Token::KEYWORD_BOOL:
                return "bool";
            case Token::KEYWORD_COMPLEX:
                return "complex";
            default:
                return "unknown_primitive";
        }
    }
    [[nodiscard]] auto clone() const -> std::unique_ptr<TypeNode> override {
        return std::make_unique<PrimitiveTypeNode>(primitive_type);
    }
};

struct PointerTypeNode : public TypeNode {
    std::unique_ptr<TypeNode> base_type;
    PointerTypeNode(std::unique_ptr<TypeNode> base)
        : TypeNode(TypeCategory::POINTER), base_type(std::move(base)) {}
    [[nodiscard]] auto typeName() const -> std::string override {
        return base_type->typeName() + "*";
    }
    [[nodiscard]] auto clone() const -> std::unique_ptr<TypeNode> override {
        return std::make_unique<PointerTypeNode>(base_type->clone());
    }
};

struct ArrayTypeNode : public TypeNode {
    std::unique_ptr<TypeNode> base_type;
    int size;

    ArrayTypeNode(std::unique_ptr<TypeNode> base, int sz)
        : TypeNode(TypeCategory::ARRAY), base_type(std::move(base)), size(sz) {}
    [[nodiscard]] auto clone() const -> std::unique_ptr<TypeNode> override {
        return std::make_unique<ArrayTypeNode>(base_type->clone(), size);
    }
    [[nodiscard]] auto typeName() const -> std::string override {
        return base_type->typeName() + "[" + std::to_string(size) + "]";
    }
};

struct StructTypeNode : public TypeNode {
    std::string struct_name;
    StructTypeNode(std::string name)
        : TypeNode(TypeCategory::STRUCT), struct_name(std::move(name)) {}
    [[nodiscard]] auto clone() const -> std::unique_ptr<TypeNode> override {
        return std::make_unique<StructTypeNode>(struct_name);
    }
    [[nodiscard]] auto typeName() const -> std::string override {
        return "struct " + struct_name;
    }
};

struct AutoTypeNode : public TypeNode {
    AutoTypeNode()
        : TypeNode(TypeCategory::PRIMITIVE) {
    }  // Treat as primitive for simplicity, actual type deduced later
    [[nodiscard]] auto clone() const -> std::unique_ptr<TypeNode> override {
        return std::make_unique<AutoTypeNode>();
    }
    [[nodiscard]] auto typeName() const -> std::string override {
        return "auto (deducing)";
    }
};

struct StructMember {
    enum class Visibility { PUBLIC, PRIVATE };

    std::unique_ptr<TypeNode> type;
    std::string name;
    int offset{};
    Visibility visibility = Visibility::PUBLIC;  // Default to public
};

struct StructDefinitionNode : public ASTNode {
    std::string name;
    std::vector<StructMember> members;
    int size{0};

    [[nodiscard]] auto type_name() const -> std::string override {
        std::string info = "STRUCT_DEF: " + name + " { ";
        for (const auto& m : members) {
            info += m.name + " ";  // Just the names for simplicity
        }
        info += "}";
        return info;
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {};
    }

    StructDefinitionNode(std::string struct_name, int line = -1, int column = -1)
        : ASTNode(NodeType::STRUCT_DEFINITION, line, column), name(std::move(struct_name)) {}

    [[nodiscard]] auto clone() const -> std::shared_ptr<StructDefinitionNode> {
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
    std::unique_ptr<ASTNode> struct_expr;  // The expression representing the struct instance
    std::string member_name;
    Symbol* resolved_symbol{nullptr};

    [[nodiscard]] auto type_name() const -> std::string override {
        return "MEMBER_ACCESS: " + member_name;
    }

    MemberAccessNode(std::unique_ptr<ASTNode> expr, std::string member, int line = -1,
                     int column = -1)
        : ASTNode(NodeType::MEMBER_ACCESS_EXPRESSION, line, column),
          struct_expr(std::move(expr)),
          member_name(std::move(member)) {}
};

struct QubitDefinitionNode : public ASTNode {
    int qubit_index;
    std::string qubit_name;

    bool has_custom_amplitudes = false;
    std::unique_ptr<ASTNode> alpha;
    std::unique_ptr<ASTNode> beta;

    [[nodiscard]] auto type_name() const -> std::string override {
        std::string info = "QUBIT_DEF: " + qubit_name + " { ";
        info += "alpha: " + (alpha ? alpha->get_value() : "default") + ", ";
        info += "beta: " + (beta ? beta->get_value() : "default");
        info += " }";
        return info;
    }

    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> children;
        if (alpha) children.push_back(alpha.get());
        if (beta) children.push_back(beta.get());
        return children;
    }

    QubitDefinitionNode(int index, std::string name, int line = -1, int column = -1)
        : ASTNode(NodeType::QUBIT_DEFINITION, line, column), 
          qubit_index(index), 
          qubit_name(std::move(name)) {}
};

struct NamespaceMember {
    std::string name;
    std::unique_ptr<ASTNode> node;
};

class SymbolTable;  // Forward declaration

struct NamespaceDefinition : public ASTNode {
    std::string name;
    std::vector<NamespaceMember> members;
    int size{};

    SymbolTable* namespace_scope{nullptr};

    [[nodiscard]] auto type_name() const -> std::string override {
        return "NAMESPACE_DEF: " + name;
    }

    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> children;
        for (const auto& m : members) {
            if (m.node) {
                children.push_back(m.node.get());
            }
        }
        return children;
    }

    NamespaceDefinition(std::string n, int line = -1, int column = -1)
        : ASTNode(NodeType::NAMESPACE_DEFINITION, line, column), name(std::move(n)) {}
};

class ScopeResolutionNode : public ASTNode {
   public:
    std::string namespace_name;
    std::unique_ptr<ASTNode> member;
    Symbol* resolved_symbol{nullptr};
    std::string mangled_name;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "SCOPE_RESOLUTION: " + namespace_name + "::";
    }

    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {member.get()};
    }

    ScopeResolutionNode(std::string ns, std::unique_ptr<ASTNode> mem, int line = -1,
                        int column = -1)
        : ASTNode(NodeType::SCOPE_RESOLUTION, line, column),  // Use SCOPE_RESOLUTION here
          namespace_name(std::move(ns)),
          member(std::move(mem)) {}

    [[nodiscard]] auto is_constant() const -> bool override {
        return member->is_constant();
    }
};

struct ExpressionNode;  // Forward declaration

struct Declaration {
    std::string name;
    std::unique_ptr<ASTNode> initial_value;
    Symbol* resolved_symbol = nullptr;
};

// Node for variable declarations (e.g., int/string x;)
struct VariableDeclarationNode : public ASTNode {
    std::unique_ptr<TypeNode> type;
    std::vector<Declaration> declarations;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "VAR_DECL";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        for (const auto& decl : declarations) {
            if (decl.initial_value) {
                refs.push_back(decl.initial_value.get());
            }
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

    VariableAssignmentNode(std::unique_ptr<ASTNode> left, std::unique_ptr<ASTNode> right,
                           int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_ASSIGNMENT, line, column),
          left(std::move(left)),
          right(std::move(right)) {}
};

// Node for variable references in expressions (e.g., x in x + 1)
struct VariableReferenceNode : public ASTNode {
    std::string name;
    Symbol* resolved_symbol{nullptr};
    int resolved_offset{0};
    std::vector<std::string> scopes;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "VAR_REF:";
    }
    [[nodiscard]] auto get_value() const -> std::string override {
        return name;
    }

    VariableReferenceNode(std::string var_name, int line = -1, int column = -1)
        : ASTNode(NodeType::VARIABLE_REFERENCE, line, column), name(std::move(var_name)) {}
};

// Node for unary operations.
struct UnaryOpExpressionNode : public ASTNode {
    Token::Type op_type;
    std::unique_ptr<ASTNode> operand;
    Symbol* resolved_symbol{nullptr};
    // std::unique_ptr<TypeNode> resolved_type;

    UnaryOpExpressionNode(Token::Type op, std::unique_ptr<ASTNode> operand_node, int line = -1,
                          int column = -1)
        : ASTNode(NodeType::UNARY_OP_EXPRESSION, line, column),
          op_type(op),
          operand(std::move(operand_node)) {}
};

struct ArrayAccessNode : public ASTNode {
    std::unique_ptr<ASTNode> array_expr;
    std::unique_ptr<ASTNode> index_expr;
    Symbol* resolved_symbol{nullptr};

    [[nodiscard]] auto type_name() const -> std::string override {
        return "ARRAY_ACCESS";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {array_expr.get(), index_expr.get()};
    }

    ArrayAccessNode(std::unique_ptr<ASTNode> array, std::unique_ptr<ASTNode> index, int line = -1,
                    int column = -1)
        : ASTNode(NodeType::ARRAY_ACCESS_EXPRESSION, line, column),
          array_expr(std::move(array)),
          index_expr(std::move(index)) {}
};

struct ParameterNode {
    std::unique_ptr<TypeNode> type;
    std::string name;
    int offset;  // Add offset for parameter
};

// Node for function definitions (e.g., int main() {})
struct FunctionDefinitionNode : public ASTNode {
    std::unique_ptr<TypeNode> return_type;
    std::string name;
    std::string mangled_name;
    std::vector<std::unique_ptr<ParameterNode>> parameters;
    std::vector<std::unique_ptr<ASTNode>> body_statements;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "FUNCTION_DEF: " + name;
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        for (const auto& stmt : body_statements) {
            refs.push_back(stmt.get());
        }
        return refs;
    }

    FunctionDefinitionNode(std::unique_ptr<TypeNode> ret_type, std::string func_name, int line = -1,
                           int column = -1)
        : ASTNode(NodeType::FUNCTION_DEFINITION, line, column),
          return_type(std::move(ret_type)),
          name(std::move(func_name)) {}
    bool is_extern{false};
};

// Node for function calls
struct FunctionCallNode : public ASTNode {
    std::string function_name;
    std::vector<std::unique_ptr<ASTNode>> arguments;
    Symbol* resolved_symbol{nullptr};

    [[nodiscard]] auto type_name() const -> std::string override {
        return "FUNC_CALL: " + function_name;
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        for (const auto& arg : arguments) {
            refs.push_back(arg.get());
        }
        return refs;
    }

    FunctionCallNode(std::string name, std::vector<std::unique_ptr<ASTNode>> args, int line = -1,
                     int column = -1)
        : ASTNode(NodeType::FUNCTION_CALL, line, column),
          function_name(std::move(name)),
          arguments(std::move(args)) {}
};

// Node for while statements
struct WhileStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> body;

    WhileStatementNode(std::unique_ptr<ASTNode> cond,
                       std::vector<std::unique_ptr<ASTNode>> body_stmts, int line = -1,
                       int column = -1)
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

    ForStatementNode(std::unique_ptr<ASTNode> init, std::unique_ptr<ASTNode> cond,
                     std::unique_ptr<ASTNode> incr,
                     std::vector<std::unique_ptr<ASTNode>> body_stmts, int line = -1,
                     int column = -1)
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

    [[nodiscard]] auto type_name() const -> std::string override {
        return "BINARY_OP: ";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {left.get(), right.get()};
    }

    BinaryOperationExpressionNode(std::unique_ptr<ASTNode> left_expr, Token::Type op,
                                  std::unique_ptr<ASTNode> right_expr, int line = -1,
                                  int column = -1)
        : ASTNode(NodeType::BINARY_OPERATION_EXPRESSION, line, column),
          left(std::move(left_expr)),
          op_type(op),
          right(std::move(right_expr)) {}
};

struct GateAppOperationExpressionNode : public ASTNode {
    std::unique_ptr<ASTNode> gate;
    Token::Type op_type;
    std::unique_ptr<ASTNode> qubit;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "GATE_APPLICATION_OP: ";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {gate.get(), qubit.get()};
    }

    GateAppOperationExpressionNode(std::unique_ptr<ASTNode> left_expr, Token::Type op,
                              std::unique_ptr<ASTNode> right_expr, int line = -1,
                              int column = -1)
    : ASTNode(NodeType::GATE_APPLICATION_OPERATION_EXPRESSION, line, column),
      gate(std::move(left_expr)),
      op_type(op),
      qubit(std::move(right_expr)) {}
};

enum OutputStream { STDOUT, STDERR };

// Node for print statements (e.g., 'print x, "hello";')
struct PrintStatementNode : public ASTNode {
    std::vector<std::unique_ptr<ASTNode>> expressions;
    OutputStream outstream;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "PRINT_STMT";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        for (const auto& expr : expressions) {
            refs.push_back(expr.get());
        }
        return refs;
    }

    PrintStatementNode(std::vector<std::unique_ptr<ASTNode>> exprs, OutputStream outs, int line = -1, int column = -1)
        : ASTNode(NodeType::PRINT_STATEMENT, line, column), expressions(std::move(exprs)), outstream(std::move(outs)) {}
};

// Node for if statements.
struct IfStatementNode : public ASTNode {
    std::unique_ptr<ASTNode> condition;
    std::vector<std::unique_ptr<ASTNode>> true_block;
    std::vector<std::unique_ptr<ASTNode>> false_block;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "IF_STATEMENT";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        if (condition) {
            refs.push_back(condition.get());
        }
        for (const auto& stmt : true_block) {
            refs.push_back(stmt.get());
        }
        for (const auto& stmt : false_block) {
            refs.push_back(stmt.get());
        }
        return refs;
    }

    IfStatementNode(std::unique_ptr<ASTNode> cond, std::vector<std::unique_ptr<ASTNode>> t_block,
                    std::vector<std::unique_ptr<ASTNode>> f_block = {}, int line = -1,
                    int column = -1)
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

    [[nodiscard]] auto type_name() const -> std::string override {
        return "SWITCH_STATEMENT";
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        if (condition) {
            refs.push_back(condition.get());
        }
        for (const auto& c : cases) {
            if (c.constant_expr) {
                refs.push_back(c.constant_expr.get());
            }
            for (const auto& stmt : c.body) {
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

    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        std::vector<ASTNode*> refs;
        for (const auto& stmt : statements) {
            refs.push_back(stmt.get());
        }
        for (const auto& func : functions) {
            refs.push_back(func.get());
        }
        for (const auto& str : structs) {
            refs.push_back(str.get());
        }
        return refs;
    }

    [[nodiscard]] auto type_name() const -> std::string override {
        return "PROGRAM_ROOT";
    }

    ProgramNode(int line = -1, int column = -1) : ASTNode(NodeType::PROGRAM, line, column) {}
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
    Symbol* resolved_symbol{nullptr};

    [[nodiscard]] auto type_name() const -> std::string override {
        return "CONST_DECL: " + name;
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {initial_value.get()};
    }

    ConstantDeclarationNode(std::string name, std::unique_ptr<TypeNode> type,
                            std::unique_ptr<ASTNode> initial_val, int line = -1, int column = -1)
        : ASTNode(NodeType::CONSTANT_DECLARATION, line, column),
          name(std::move(name)),
          type(std::move(type)),
          initial_value(std::move(initial_val)) {}
};

struct EnumMemberNode {
    std::string name;
    std::unique_ptr<ASTNode> value;  // Can be nullptr for implicit values

    EnumMemberNode(std::string name, std::unique_ptr<ASTNode> value = nullptr)
        : name(std::move(name)), value(std::move(value)) {}
};

struct EnumStatementNode : public ASTNode {
    std::string name;
    std::vector<std::unique_ptr<EnumMemberNode>> members;

    [[nodiscard]] auto type_name() const -> std::string override {
        return "ENUM: " + name;
    }
    [[nodiscard]] auto get_children() const -> std::vector<ASTNode*> override {
        return {};
    }

    EnumStatementNode(std::string name, std::vector<std::unique_ptr<EnumMemberNode>> members,
                      int line = -1, int column = -1)
        : ASTNode(NodeType::ENUM_STATEMENT, line, column),
          name(std::move(name)),
          members(std::move(members)) {}
};

#endif  // AST_HPP
