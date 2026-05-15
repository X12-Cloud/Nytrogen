#ifndef SEMANTIC_ANALYZER_HPP
#define SEMANTIC_ANALYZER_HPP

#include "utils.hpp"
#include <memory>
#include <string>
#include <vector>
#include <map>
#include "ast.hpp"
#include "symbol_table.hpp"

class SemanticAnalyzer {
public:
    SemanticAnalyzer(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable)
        : program_ast(ast), symbolTable(symTable) {}

    void analyze();
    SymbolTable& getSymbolTable() { return symbolTable; }
    int getTypeSize(const TypeNode* type); // Helper to get size of a type
    bool areTypesCompatible(const TypeNode* type1, const TypeNode* type2);

    void setIsEntryPoint(bool entry) { is_entry_point = entry; }
    bool debug_mode = false;

private:
    bool is_entry_point = false;
    std::unique_ptr<ProgramNode>& program_ast;
    SymbolTable& symbolTable;
    std::string typeToString(const TypeNode* type);
    TypeNode* currentFunctionReturnType = nullptr;
    std::vector<std::string> namespace_stack;

    // Visitor methods for AST nodes
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
    void visit(NamespaceDefinition* node);
    void visit(AsmStatementNode* node);
    void visit(ConstantDeclarationNode* node);
    void visit(EnumStatementNode* node);
    void visit(SwitchStatementNode* node);
    void visit(ScopeResolutionNode* node);

    // Expression visitors (return the type of the expression)
    std::unique_ptr<TypeNode> visitExpression(ASTNode* expr);
    std::unique_ptr<TypeNode> visitIntegerLiteralExpression(IntegerLiteralExpressionNode* node);
    std::unique_ptr<TypeNode> visitStringLiteralExpression(StringLiteralExpressionNode* node);
    std::unique_ptr<TypeNode> visitBooleanLiteralExpression(BooleanLiteralExpressionNode* node);
    std::unique_ptr<TypeNode> visitCharacterLiteralExpression(CharacterLiteralExpressionNode* node);
    std::unique_ptr<TypeNode> visitFloatLiteralExpression(FloatLiteralExpressionNode* node);
    std::unique_ptr<TypeNode> visitDoubleLiteralExpression(DoubleLiteralExpressionNode* node);
};

#endif // SEMANTIC_ANALYZER_HPP
