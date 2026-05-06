#ifndef CODE_GENERATOR_HPP
#define CODE_GENERATOR_HPP

#include "utils.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ast.hpp"
#include "symbol_table.hpp"

struct GlobalConstant {
    std::string label;
    std::string type;
    std::string value;
};

class CodeGenerator {
public:
    CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable);
    void generate(const std::string& output_filename, bool is_entry_point);
    bool isFloatingPoint(const std::shared_ptr<TypeNode>& type);

private:
    std::vector<GlobalConstant> constants;
    std::map<std::string, std::string> constants_map;
    int string_label_counter;
    std::string current_function_name;
    std::string current_namespace_name;

    std::unique_ptr<ProgramNode>& program_ast;
    SymbolTable& symbolTable;
    std::ofstream out;

    void visit(ASTNode* node);
    void visit(ProgramNode* node);
    void visit(FunctionDefinitionNode* node);
    void visit(VariableDeclarationNode* node);
    void visit(VariableAssignmentNode* node);
    void visit(VariableReferenceNode* node);
    void visit(BinaryOperationExpressionNode* node);
    void visit(PrintStatementNode* node);
    void visit(ReturnStatementNode* node);
    void visit(IfStatementNode* node);
    void visit(SwitchStatementNode* node);
    void visit(WhileStatementNode* node);
    void visit(ForStatementNode* node);
    void visit(FunctionCallNode* node);
    void visit(MemberAccessNode* node);
    void visit(UnaryOpExpressionNode* node);
    void visit(ArrayAccessNode* node);
    void visit(StructDefinitionNode* node);
    void visit(IntegerLiteralExpressionNode* node);
    void visit(StringLiteralExpressionNode* node);
    void visit(BooleanLiteralExpressionNode* node);
    void visit(CharacterLiteralExpressionNode* node);
    void visit(AsmStatementNode* node);
    void visit(ConstantDeclarationNode* node);
    void visit(EnumStatementNode* node);
    void visit(DoubleLiteralExpressionNode* node);
    void visit(FloatLiteralExpressionNode* node);
    void visit(NamespaceDefinition* node);
    void visit(ScopeResolutionNode* node);

    int getTypeSize(const TypeNode* type);

    // Instruction set
    void emit(const std::string& instr);
    void emit(const std::string& instr, const std::string reg);
    void emit(const std::string& instr, const std::string& dest, const std::string& src);
    void emit_adv(const std::shared_ptr<TypeNode>& type, const std::string& base_reg, int offset, const std::string& src_val);
    void emit_adv(const std::unique_ptr<TypeNode>& type, const std::string& base_reg, int offset, const std::string& src_val);
    void emit_binary_op(const std::string& op_instr, char type);
    void emit_print(const std::shared_ptr<TypeNode>& type);
    void load_adv(const std::shared_ptr<TypeNode>& type, const std::string& dest_reg, const std::string base_reg, int offset);
    void load_adv(const std::unique_ptr<TypeNode>& type, const std::string& dest_reg, const std::string& base_reg, int offset);
};

#endif // CODE_GENERATOR_HPP
