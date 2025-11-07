#include "semantic_analyzer.hpp"
#include <iostream>
#include <stdexcept>

// Helper to get size of a type
int SemanticAnalyzer::getTypeSize(const TypeNode* type) {
    if (!type) {
        throw std::runtime_error("Semantic Error: Attempted to get size of a null type.");
    }

    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const PrimitiveTypeNode* prim_type = static_cast<const PrimitiveTypeNode*>(type);
            switch (prim_type->primitive_type) {
                case Token::KEYWORD_INT: return 4; // 4 bytes for int (dword)
                case Token::KEYWORD_BOOL: return 1; // 1 byte for bool
                case Token::KEYWORD_CHAR: return 1; // 1 byte for char
                case Token::KEYWORD_STRING: return 8; // 8 bytes for string (pointer)
                case Token::KEYWORD_VOID: return 0; // Void has no size
                default: throw std::runtime_error("Semantic Error: Unknown primitive type for size calculation.");
            }
        }
        case TypeNode::TypeCategory::POINTER:
            return 8; // Pointers are 8 bytes on x64
        case TypeNode::TypeCategory::ARRAY: {
            const ArrayTypeNode* array_type = static_cast<const ArrayTypeNode*>(type);
            if (array_type->size <= 0) {
                throw std::runtime_error("Semantic Error: Unsized arrays not allowed for local variables.");
            }
            int element_size = getTypeSize(array_type->base_type.get());
            if (array_type->size > 0) {
                return element_size * array_type->size;
            }
            return 0; 
        }
        case TypeNode::TypeCategory::STRUCT: {
            const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(type);
            Symbol* struct_def_symbol = symbolTable.lookup(struct_type->struct_name);
            if (!struct_def_symbol || !struct_def_symbol->structDef) {
                throw std::runtime_error("Semantic Error: Undefined struct '" + struct_type->struct_name + "'.");
            }
            return struct_def_symbol->structDef->size;
        }
        default:
            throw std::runtime_error("Semantic Error: Unknown type category for size calculation.");
    }
}

bool SemanticAnalyzer::areTypesCompatible(const TypeNode* type1, const TypeNode* type2) {
    if (!type1 || !type2) {
        return false; // Null types are not compatible
    }

    if (type1->category != type2->category) {
        return false;
    }

    switch (type1->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const PrimitiveTypeNode* p1 = static_cast<const PrimitiveTypeNode*>(type1);
            const PrimitiveTypeNode* p2 = static_cast<const PrimitiveTypeNode*>(type2);
            return p1->primitive_type == p2->primitive_type;
        }
        case TypeNode::TypeCategory::POINTER: {
            const PointerTypeNode* ptr1 = static_cast<const PointerTypeNode*>(type1);
            const PointerTypeNode* ptr2 = static_cast<const PointerTypeNode*>(type2);
            return areTypesCompatible(ptr1->base_type.get(), ptr2->base_type.get());
        }
        case TypeNode::TypeCategory::ARRAY: {
            const ArrayTypeNode* arr1 = static_cast<const ArrayTypeNode*>(type1);
            const ArrayTypeNode* arr2 = static_cast<const ArrayTypeNode*>(type2);
            return areTypesCompatible(arr1->base_type.get(), arr2->base_type.get());
        }
        case TypeNode::TypeCategory::STRUCT: {
            const StructTypeNode* s1 = static_cast<const StructTypeNode*>(type1);
            const StructTypeNode* s2 = static_cast<const StructTypeNode*>(type2);
            return s1->struct_name == s2->struct_name;
        }
        default:
            return false; // Unknown category
    }
}

void SemanticAnalyzer::analyze() {
    symbolTable.enterScope(); // global scope

    // Process structs
    for (const auto& struct_node : program_ast->structs) {
        visit(struct_node.get());
    }

    // Declare functions (but don't visit bodies yet)
    for (const auto& func_node : program_ast->functions) {
        std::unique_ptr<TypeNode> return_type = func_node->return_type->clone();
        std::vector<std::unique_ptr<TypeNode>> param_types;
        for (const auto& param : func_node->parameters) {
            param_types.push_back(param->type->clone());
        }
        Symbol func_symbol(Symbol::SymbolType::FUNCTION, std::string(func_node->name), std::move(return_type), std::move(param_types));
        symbolTable.addSymbol(std::move(func_symbol));
    }

    // Process global statements
    for (const auto& stmt : program_ast->statements) {
        visit(stmt.get());
    }

    // Now visit function bodies — but do NOT exit their scopes
    for (const auto& func_node : program_ast->functions) {
        visit(func_node.get());
        // DO NOT exit scope — code generator needs it
    }

    // Check for main
    bool has_main = false;
    for (const auto& func : program_ast->functions) {
        if (func->name == "main") {
            has_main = true;
            if (func->return_type->category != TypeNode::TypeCategory::PRIMITIVE ||
                static_cast<PrimitiveTypeNode*>(func->return_type.get())->primitive_type != Token::KEYWORD_INT) {
                throw std::runtime_error("Semantic Error: 'main' function must return int.");
                }
                if (!func->parameters.empty()) {
                    throw std::runtime_error("Semantic Error: 'main' function should have no parameters.");
                }
        }
    }
    if (!has_main) {
        throw std::runtime_error("Semantic Error: No 'main' function defined.");
    }

    // DO NOT exit global scope — code generator might need it
    // symbolTable.exitScope(); ← COMMENTED OUT
}

void SemanticAnalyzer::visit(ASTNode* node) {
    if (!node) return;

    switch (node->node_type) {
        case ASTNode::NodeType::PROGRAM:
            visit(static_cast<ProgramNode*>(node));
            break;
        case ASTNode::NodeType::FUNCTION_DEFINITION:
            visit(static_cast<FunctionDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::VARIABLE_DECLARATION:
            visit(static_cast<VariableDeclarationNode*>(node));
            break;
        case ASTNode::NodeType::VARIABLE_ASSIGNMENT:
            visit(static_cast<VariableAssignmentNode*>(node));
            break;
        case ASTNode::NodeType::VARIABLE_REFERENCE:
            visit(static_cast<VariableReferenceNode*>(node));
            break;
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION:
            visit(static_cast<BinaryOperationExpressionNode*>(node));
            break;
        case ASTNode::NodeType::PRINT_STATEMENT:
            visit(static_cast<PrintStatementNode*>(node));
            break;
        case ASTNode::NodeType::RETURN_STATEMENT:
            visit(static_cast<ReturnStatementNode*>(node));
            break;
        case ASTNode::NodeType::IF_STATEMENT:
            visit(static_cast<IfStatementNode*>(node));
            break;
        case ASTNode::NodeType::WHILE_STATEMENT:
            visit(static_cast<WhileStatementNode*>(node));
            break;
        case ASTNode::NodeType::FOR_STATEMENT:
            visit(static_cast<ForStatementNode*>(node));
            break;
        case ASTNode::NodeType::FUNCTION_CALL:
            visit(static_cast<FunctionCallNode*>(node));
            break;
        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION:
            visit(static_cast<MemberAccessNode*>(node));
            break;
        case ASTNode::NodeType::UNARY_OP_EXPRESSION:
            visit(static_cast<UnaryOpExpressionNode*>(node));
            break;
        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION:
            visit(static_cast<ArrayAccessNode*>(node));
            break;
        case ASTNode::NodeType::STRUCT_DEFINITION:
            visit(static_cast<StructDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION:
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            break;
        case ASTNode::NodeType::ASM_STATEMENT:
            visit(static_cast<AsmStatementNode*>(node));
            break;
        default:
            throw std::runtime_error("Semantic Error: Unknown AST node type encountered during analysis.");
    }
}

void SemanticAnalyzer::visit(ProgramNode* node) {
}

void SemanticAnalyzer::visit(FunctionDefinitionNode* node) {
    symbolTable.enterScope();
    int param_offset = 16;
    for (const auto& param : node->parameters) {
        int size = getTypeSize(param->type.get());
        symbolTable.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, param->name, param->type->clone(), param_offset, size));
        param_offset += size;
    }
    symbolTable.scopes.back()->currentOffset = 0;

    // Analyze body statements — variables declared here
    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    // DO NOT exit scope here — code generator needs it!
    // symbolTable.exitScope(); ← COMMENTED OUT
}

void SemanticAnalyzer::visit(VariableDeclarationNode* node) {
    if (symbolTable.scopes.back()->lookup(node->name)) {
        throw std::runtime_error("Semantic Error: Redefinition of variable '" + node->name + "'.");
    }

    int var_size = getTypeSize(node->type.get());

    symbolTable.scopes.back()->currentOffset -= var_size;
    int offset = symbolTable.scopes.back()->currentOffset;

    Symbol symbol(Symbol::SymbolType::VARIABLE, node->name, node->type->clone(), offset, var_size);
    node->resolved_symbol = symbolTable.addSymbol(std::move(symbol));

    if (node->initial_value) {
        std::unique_ptr<TypeNode> expr_type = visitExpression(node->initial_value.get());
        if (!areTypesCompatible(expr_type.get(), node->type.get())) {
            throw std::runtime_error("Semantic Error: Type mismatch in variable initialization for '" + node->name + "'.");
        }
    }
}

void SemanticAnalyzer::visit(VariableAssignmentNode* node) {
    std::unique_ptr<TypeNode> left_type = visitExpression(node->left.get());
    std::unique_ptr<TypeNode> right_type = visitExpression(node->right.get());

    if (!areTypesCompatible(left_type.get(), right_type.get())) {
        throw std::runtime_error("Semantic Error: Type mismatch in assignment.");
    }
}

void SemanticAnalyzer::visit(VariableReferenceNode* node) {
    Symbol* var_symbol = symbolTable.lookup(node->name);
    if (!var_symbol) {
        throw std::runtime_error("Semantic Error: Use of undeclared variable '" + node->name + "'.");
    }
    node->resolved_symbol = var_symbol;
    node->resolved_offset = var_symbol->offset;
    node->resolved_type = var_symbol->dataType->clone();
}

void SemanticAnalyzer::visit(BinaryOperationExpressionNode* node) {
    std::unique_ptr<TypeNode> left_type = visitExpression(node->left.get());
    std::unique_ptr<TypeNode> right_type = visitExpression(node->right.get());

    if (left_type->category != right_type->category) {
        throw std::runtime_error("Semantic Error: Type mismatch in binary operation.");
    }

    switch (node->op_type) {
        case Token::EQUAL_EQUAL:
        case Token::BANG_EQUAL:
        case Token::LESS:
        case Token::GREATER:
        case Token::LESS_EQUAL:
        case Token::GREATER_EQUAL:
            node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
            break;
        default:
            node->resolved_type = std::move(left_type);
            break;
    }
}

void SemanticAnalyzer::visit(PrintStatementNode* node) {
    for (const auto& expr : node->expressions) {
        expr->resolved_type = visitExpression(expr.get());
    }
}

void SemanticAnalyzer::visit(ReturnStatementNode* node) {
    visitExpression(node->expression.get());
}

void SemanticAnalyzer::visit(IfStatementNode* node) {
    std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
    if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_BOOL) {
        throw std::runtime_error("Semantic Error: If condition must be a boolean expression.");
    }

    for (const auto& stmt : node->true_block) {
        visit(stmt.get());
    }
    for (const auto& stmt : node->false_block) {
        visit(stmt.get());
    }
}

void SemanticAnalyzer::visit(WhileStatementNode* node) {
    std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
    if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_BOOL) {
        throw std::runtime_error("Semantic Error: While condition must be a boolean expression.");
    }

    symbolTable.enterScope();
    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }
    symbolTable.exitScope();
}

void SemanticAnalyzer::visit(ForStatementNode* node) {
    symbolTable.enterScope(); // Scope for initializer, condition, increment, and body

    if (node->initializer) {
        visit(node->initializer.get());
    }
    if (node->condition) {
        std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
        if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
            static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_BOOL) {
            throw std::runtime_error("Semantic Error: For loop condition must be a boolean expression.");
        }
    }
    if (node->increment) {
        visit(node->increment.get());
    }
    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }

    symbolTable.exitScope();
}

void SemanticAnalyzer::visit(FunctionCallNode* node) {
    Symbol* func_symbol = symbolTable.lookup(node->function_name);
    if (!func_symbol || func_symbol->type != Symbol::SymbolType::FUNCTION) {
        throw std::runtime_error("Semantic Error: Call to undeclared function '" + node->function_name + "'.");
    }
    node->resolved_symbol = func_symbol;

    // Check number of arguments
    if (node->arguments.size() != func_symbol->parameterTypes.size()) {
        throw std::runtime_error("Semantic Error: Function '" + node->function_name + "' expects " +
                                 std::to_string(func_symbol->parameterTypes.size()) + " arguments, but " +
                                 std::to_string(node->arguments.size()) + " were provided.");
    }

    // Check argument types
    for (size_t i = 0; i < node->arguments.size(); ++i) {
        std::unique_ptr<TypeNode> arg_type = visitExpression(node->arguments[i].get());
        if (!areTypesCompatible(arg_type.get(), func_symbol->parameterTypes[i].get())) {
            throw std::runtime_error("Semantic Error: Type mismatch in argument " + std::to_string(i + 1) +
                                     " of function '" + node->function_name + "'.");
        }
    }
}

void SemanticAnalyzer::visit(MemberAccessNode* node) {
    std::unique_ptr<TypeNode> base_type = visitExpression(node->struct_expr.get());

    if (base_type->category != TypeNode::TypeCategory::STRUCT) {
        throw std::runtime_error("Semantic Error: Member access operator '.' used on non-struct type.");
    }

    const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(base_type.get());
    Symbol* struct_def_symbol = symbolTable.lookup(struct_type->struct_name);

    if (!struct_def_symbol || !struct_def_symbol->structDef) {
        throw std::runtime_error("Semantic Error: Undefined struct '" + struct_type->struct_name + "'.");
    }

    const auto& struct_def = struct_def_symbol->structDef;

    bool member_found = false;
    for (const auto& member : struct_def->members) {
        if (member.name == node->member_name) {
            member_found = true;
            node->resolved_symbol = symbolTable.lookup(member.name);
            break;
        }
    }

    if (!member_found) {
        throw std::runtime_error("Semantic Error: Struct '" + struct_type->struct_name + "' has no member named '" + node->member_name + "'.");
    }
}

void SemanticAnalyzer::visit(UnaryOpExpressionNode* node) {
    std::unique_ptr<TypeNode> operand_type = visitExpression(node->operand.get());

    if (node->op_type == Token::ADDRESSOF) {
        if (node->operand->node_type != ASTNode::NodeType::VARIABLE_REFERENCE) {
            throw std::runtime_error("Semantic Error: Address-of operator '&' can only be applied to variables.");
        }
        node->resolved_symbol = static_cast<VariableReferenceNode*>(node->operand.get())->resolved_symbol;
        node->resolved_type = std::make_unique<PointerTypeNode>(std::move(operand_type));
    } else if (node->op_type == Token::STAR) {
        if (operand_type->category != TypeNode::TypeCategory::POINTER) {
            throw std::runtime_error("Semantic Error: Dereference operator '*' can only be applied to pointer types.");
        }
        node->resolved_type = static_cast<PointerTypeNode*>(operand_type.get())->base_type->clone();
    } else {
        throw std::runtime_error("Semantic Error: Unknown unary operator.");
    }
}

void SemanticAnalyzer::visit(ArrayAccessNode* node) {
    std::unique_ptr<TypeNode> array_type = visitExpression(node->array_expr.get());
    std::unique_ptr<TypeNode> index_type = visitExpression(node->index_expr.get());

    if (array_type->category != TypeNode::TypeCategory::ARRAY) {
        throw std::runtime_error("Semantic Error: Array access operator '[]' used on non-array type.");
    }

    if (index_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(index_type.get())->primitive_type != Token::KEYWORD_INT) {
        throw std::runtime_error("Semantic Error: Array index must be an integer.");
    }
    node->resolved_symbol = new Symbol(Symbol::SymbolType::VARIABLE, "", static_cast<ArrayTypeNode*>(array_type.get())->base_type->clone(), 0, getTypeSize(static_cast<ArrayTypeNode*>(array_type.get())->base_type.get()));
}

void SemanticAnalyzer::visit(StructDefinitionNode* node) {
    // Calculate offsets
    int offset = 0;
    for (auto& member : node->members) {
        member.offset = offset;
        offset += getTypeSize(member.type.get());
    }
    node->size = offset;

    // Register in symbol table
    symbolTable.addSymbol(Symbol(Symbol::SymbolType::STRUCT_DEFINITION, node->name, node->clone()));
}

void SemanticAnalyzer::visit(AsmStatementNode* node) {
    // No semantic analysis needed for inline assembly
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitExpression(ASTNode* expr) {
    if (!expr) {
        throw std::runtime_error("Semantic Error: Attempted to visit a null expression.");
    }

    switch (expr->node_type) {
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION:
            return visitIntegerLiteralExpression(static_cast<IntegerLiteralExpressionNode*>(expr));
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
            return visitStringLiteralExpression(static_cast<StringLiteralExpressionNode*>(expr));
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
            return visitBooleanLiteralExpression(static_cast<BooleanLiteralExpressionNode*>(expr));
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            return visitCharacterLiteralExpression(static_cast<CharacterLiteralExpressionNode*>(expr));
        case ASTNode::NodeType::VARIABLE_REFERENCE: {
            visit(static_cast<VariableReferenceNode*>(expr));
            Symbol* sym = symbolTable.lookup(static_cast<VariableReferenceNode*>(expr)->name);
            if (!sym) throw std::runtime_error("Semantic Error: Variable '" + static_cast<VariableReferenceNode*>(expr)->name + "' not found.");
            return sym->dataType->clone();
        }
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            visit(static_cast<BinaryOperationExpressionNode*>(expr));
            return static_cast<BinaryOperationExpressionNode*>(expr)->resolved_type->clone();
        }
        case ASTNode::NodeType::FUNCTION_CALL: {
            visit(static_cast<FunctionCallNode*>(expr));
            Symbol* func_symbol = symbolTable.lookup(static_cast<FunctionCallNode*>(expr)->function_name);
            if (!func_symbol) {
                throw std::runtime_error("Semantic Error: Function '" + static_cast<FunctionCallNode*>(expr)->function_name + "' not found.");
            }
            return func_symbol->dataType->clone();
        }
        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION: {
            visit(static_cast<MemberAccessNode*>(expr));
            std::unique_ptr<TypeNode> base_type = visitExpression(static_cast<MemberAccessNode*>(expr)->struct_expr.get());
            const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(base_type.get());
            Symbol* struct_def_symbol = symbolTable.lookup(struct_type->struct_name);
            if (!struct_def_symbol || !struct_def_symbol->structDef) {
                throw std::runtime_error("Semantic Error: Undefined struct '" + struct_type->struct_name + "'.");
            }
            const auto& struct_def = struct_def_symbol->structDef;
            for (const auto& member : struct_def->members) {
                if (member.name == static_cast<MemberAccessNode*>(expr)->member_name) {
                    return member.type->clone();
                }
            }
            throw std::runtime_error("Semantic Error: Member '" + static_cast<MemberAccessNode*>(expr)->member_name + "' not found in struct '" + struct_type->struct_name + "'.");
        }
        case ASTNode::NodeType::UNARY_OP_EXPRESSION: {
            visit(static_cast<UnaryOpExpressionNode*>(expr));
            const UnaryOpExpressionNode* unary_node = static_cast<const UnaryOpExpressionNode*>(expr);
            return unary_node->resolved_type->clone();
        }
        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION: {
            visit(static_cast<ArrayAccessNode*>(expr));
            std::unique_ptr<TypeNode> array_type = visitExpression(static_cast<ArrayAccessNode*>(expr)->array_expr.get());
            const ArrayTypeNode* arr_type = static_cast<const ArrayTypeNode*>(array_type.get());
            return arr_type->base_type->clone();
        }
        case ASTNode::NodeType::VARIABLE_ASSIGNMENT: {
            auto* assign_node = static_cast<VariableAssignmentNode*>(expr);
            visit(assign_node);
            return visitExpression(assign_node->left.get());
        }
        case ASTNode::NodeType::VARIABLE_DECLARATION: {
            auto* decl_node = static_cast<VariableDeclarationNode*>(expr);
            visit(decl_node);
            return decl_node->type->clone();
        }
        default:
            throw std::runtime_error("Semantic Error: Unexpected AST node type in visitExpression.");
    }
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitIntegerLiteralExpression(IntegerLiteralExpressionNode* node) {
    return std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitStringLiteralExpression(StringLiteralExpressionNode* node) {
    return std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_STRING);
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitBooleanLiteralExpression(BooleanLiteralExpressionNode* node) {
    return std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitCharacterLiteralExpression(CharacterLiteralExpressionNode* node) {
    return std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_CHAR);
}
