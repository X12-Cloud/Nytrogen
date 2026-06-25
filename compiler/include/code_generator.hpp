#ifndef CODE_GENERATOR_HPP
#define CODE_GENERATOR_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "instruction_set.hpp"
#include "register_allocator.hpp"
#include "symbol_table.hpp"
#include "utils.hpp"

struct GlobalConstant {
    std::string label;
    std::string type;
    std::string value;
};

class CodeGenerator {
    InstructionSet emitter;

   public:
    CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable);
    void generate(const std::string& output_filename, bool is_entry_point);
    bool isFloatingPoint(const TypeNode* type);
    bool debug_mode = false;

   private:
    RegisterAllocator allocator;

    std::vector<GlobalConstant> constants;
    std::map<std::string, std::string> constants_map;
    int string_label_counter;
    std::string current_function_name;
    std::string current_namespace_name;
    int current_stack_depth;

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
    std::string getRegisterName(const std::string& reg64, int size);
    std::string reg_to_str(RegisterAllocator::RegID reg, const TypeNode* type);
};

#endif  // CODE_GENERATOR_HPP
