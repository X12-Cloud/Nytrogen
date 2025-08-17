
#ifndef CODE_GENERATOR_HPP
#define CODE_GENERATOR_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include "ast.hpp"
#include "symbol_table.hpp"

class CodeGenerator {
public:
    CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable);
    void generate(const std::string& output_filename);

private:
    std::unique_ptr<ProgramNode>& program_ast;
    SymbolTable& symbolTable;
    std::ofstream out;
    std::map<std::string, std::string> string_literals;
    int string_label_counter;

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
};

#endif // CODE_GENERATOR_HPP
