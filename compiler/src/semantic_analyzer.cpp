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
                case Token::KEYWORD_FLOAT: return 4; // 4 bytes for float (dword)
                case Token::KEYWORD_DOUBLE: return 8; // 8 bytes for double (qword)
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
            if (!symbolTable.isStructDefined(struct_type->struct_name)) {
                throw std::runtime_error("Semantic Error: Undefined struct '" + struct_type->struct_name + "'.");
            }
            return symbolTable.getStructDefinitions()[struct_type->struct_name]->size;
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
    if (is_entry_point && !has_main) {
        throw std::runtime_error("Semantic Error: No 'main' function defined.");
    }

    symbolTable.exitScope();
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
	case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION:
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION:
            break;
        case ASTNode::NodeType::ASM_STATEMENT:
            visit(static_cast<AsmStatementNode*>(node));
            break;
        case ASTNode::NodeType::CONSTANT_DECLARATION:
            visit(static_cast<ConstantDeclarationNode*>(node));
            break;
        case ASTNode::NodeType::ENUM_STATEMENT:
            visit(static_cast<EnumStatementNode*>(node));
            break;
        default:
            throw std::runtime_error("Semantic Error: Unknown AST node type encountered during analysis.");
    }
}

void SemanticAnalyzer::visit(ProgramNode* node) {
}

void SemanticAnalyzer::visit(FunctionDefinitionNode* node) {
    symbolTable.enterScope();

    currentFunctionReturnType = nullptr;

    const std::vector<std::string> arg_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int param_offset = 16; 
    int register_param_offset = 0;

    for (int i = 0; i < node->parameters.size(); ++i) {
        const auto& param = node->parameters[i];
        int size = getTypeSize(param->type.get());
        if (i < arg_registers.size()) {
            register_param_offset -= 8; 
            // Use symbolTable.addSymbol directly, it now targets current_scope
            symbolTable.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, param->name, param->type->clone(), register_param_offset, size));
        } else {
            symbolTable.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, param->name, param->type->clone(), param_offset, size));
            param_offset += size;
        }
    }
    
    // Update the offset of the CURRENT scope
    symbolTable.current_scope->currentOffset = register_param_offset;

    currentFunctionReturnType = node->return_type.get();

    if (!node->is_extern) {
        for (const auto& stmt : node->body_statements) {
            visit(stmt.get());
        }
    }

    currentFunctionReturnType = nullptr;
    symbolTable.exitScope(); 
}

void SemanticAnalyzer::visit(VariableDeclarationNode* node) {
    bool is_auto = (dynamic_cast<AutoTypeNode*>(node->type.get()) != nullptr);

    for (auto& decl : node->declarations) {
        if (symbolTable.current_scope->lookup(decl.name)) {
            throw std::runtime_error("Semantic Error: Redefinition of variable '" + decl.name + "'.");
        }

        std::unique_ptr<TypeNode> actual_type;

        /* if (decl.initial_value) {
            visitExpression(decl.initial_value.get());
        }

        int var_size = getTypeSize(actual_type.get());
        symbolTable.current_scope->currentOffset -= var_size;

        Symbol symbol(Symbol::SymbolType::VARIABLE, decl.name, actual_type->clone(), symbolTable.current_scope->currentOffset, var_size);

        decl.resolved_symbol = symbolTable.addSymbol(std::move(symbol)); */

        if (is_auto) {
            if (!decl.initial_value) {
                throw std::runtime_error("Semantic Error: 'auto' variable '" + decl.name + "' requires an initializer.");
            }
            actual_type = visitExpression(decl.initial_value.get());
            if (!actual_type) {
                throw std::runtime_error("Semantic Error: Could not deduce type for 'auto' variable '" + decl.name + "'.");
            }
            // node->type = actual_type->clone(); // Update the node's type with the deduced type
        } else {
            actual_type = node->type->clone();
            if (decl.initial_value) {
                auto expr_type = visitExpression(decl.initial_value.get());
                if (!areTypesCompatible(expr_type.get(), actual_type.get())) throw std::runtime_error("Type mismatch for '" + decl.name + "'");
            }
        }

        if (!actual_type) {
            throw std::runtime_error("Semantic Error: Type deduction failed for '" + decl.name + "'.");
        }

        int var_size = getTypeSize(actual_type.get());
        symbolTable.current_scope->currentOffset -= var_size;
        int offset = symbolTable.current_scope->currentOffset;

        Symbol symbol(Symbol::SymbolType::VARIABLE, decl.name, actual_type->clone(), offset, var_size);
        decl.resolved_symbol = symbolTable.addSymbol(std::move(symbol));
    }
}

void SemanticAnalyzer::visit(VariableAssignmentNode* node) {
    if (node->left->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
        auto* var_ref = static_cast<VariableReferenceNode*>(node->left.get());
        Symbol* symbol = symbolTable.lookup(var_ref->name);
        if (symbol && symbol->type == Symbol::SymbolType::CONSTANT) {
            throw std::runtime_error("Semantic Error: Cannot assign to constant '" + var_ref->name + "'.");
        }
    }

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
        throw std::runtime_error("Semantic Error: Type mismatch in binary operation (cannot operate on " + typeToString(left_type.get()) + " and " + typeToString(right_type.get()) + ")");
    }

    if (left_type->category == TypeNode::TypeCategory::PRIMITIVE) {
	auto p1 = static_cast<PrimitiveTypeNode*>(left_type.get());
        auto p2 = static_cast<PrimitiveTypeNode*>(right_type.get());
        if (p1->primitive_type != p2->primitive_type) {
	    throw std::runtime_error("Semantic Error: Mixed math. Adding different primitive types is not yet supported.");
        }
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
        expr->resolved_type = std::move(visitExpression(expr.get()));
        if (!expr->resolved_type) throw std::runtime_error("Semantic Error: Could not resolve type for print expression.");
    }
}

void SemanticAnalyzer::visit(ReturnStatementNode* node) {
    if (node->expression) {
        node->resolved_type = visitExpression(node->expression.get());

        if (currentFunctionReturnType->category == TypeNode::TypeCategory::PRIMITIVE) {
            auto prim = static_cast<PrimitiveTypeNode*>(currentFunctionReturnType);
            if (prim->primitive_type == Token::KEYWORD_VOID) {
                throw std::runtime_error("Semantic Error: Cannot return a value from a void function.");
            }
            if (!areTypesCompatible(node->resolved_type.get(), currentFunctionReturnType)) {
                throw std::runtime_error("Semantic Error: Return type mismatch in function.");
            }
        }
    } else {
        node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_VOID);

        bool isFunctionVoid = false;
        if (currentFunctionReturnType->category == TypeNode::TypeCategory::PRIMITIVE) {
            auto prim = static_cast<PrimitiveTypeNode*>(currentFunctionReturnType);
            if (prim->primitive_type == Token::KEYWORD_VOID) {
                isFunctionVoid = true;
            }
        }
        if (!isFunctionVoid) {
            throw std::runtime_error("Semantic Error: Non-void function must return a value.");
        }
    }
}

void SemanticAnalyzer::visit(IfStatementNode* node) {
    std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
    if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_BOOL && static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::INTEGER_LITERAL) {
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
        node->arguments[i]->resolved_type = arg_type->clone();
        if (!areTypesCompatible(arg_type.get(), func_symbol->parameterTypes[i].get())) {
            throw std::runtime_error("Semantic Error: Type mismatch in argument " + std::to_string(i + 1) +
                                     " of function '" + node->function_name + "'.");
        }
    }
    if (func_symbol->dataType) node->resolved_type = func_symbol->dataType->clone();
    else throw std::runtime_error("Semantic Error: Function '" + node->function_name + "' has no return type.");
}


// TODO: Structs dont work for some reason, "member X not found in struct Y". 
void SemanticAnalyzer::visit(MemberAccessNode* node) {
    std::unique_ptr<TypeNode> base_type = visitExpression(node->struct_expr.get());

    if (base_type->category != TypeNode::TypeCategory::STRUCT) {
        throw std::runtime_error("Semantic Error: Member access operator '.' used on non-struct type.");
    }

    const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(base_type.get());
    if (!symbolTable.isStructDefined(struct_type->struct_name)) {
        throw std::runtime_error("Semantic Error: Undefined struct '" + struct_type->struct_name + "'.");
    }

    const auto& struct_def = symbolTable.getStructDefinitions()[struct_type->struct_name];

    bool member_found = false;
    for (const auto& member : struct_def->members) {
        if (member.name == node->member_name) {
            member_found = true;

            // Check visibility
            if (member.visibility == StructMember::Visibility::PRIVATE) {
                // A more complex check would be needed for friend classes or member functions
                throw std::runtime_error("Semantic Error: Cannot access private member '" + node->member_name + "' of struct '" + struct_type->struct_name + "'.");
            }

            node->resolved_symbol = new Symbol(Symbol::SymbolType::STRUCT_MEMBER, member.name, member.type->clone(), member.offset, getTypeSize(member.type.get()), member.visibility);
	    node->resolved_type = member.type->clone();
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
    } else if (node->op_type == Token::BANG) {
	node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::INTEGER_LITERAL);
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
    node->resolved_type = static_cast<ArrayTypeNode*>(array_type.get())->base_type->clone();
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
    symbolTable.addStructDefinition(node->name, node);
}

void SemanticAnalyzer::visit(AsmStatementNode* node) {
    // No semantic analysis needed for inline assembly
}

void SemanticAnalyzer::visit(ConstantDeclarationNode* node) {
    if (symbolTable.current_scope->lookup(node->name)) {
        throw std::runtime_error("Semantic Error: Redefinition of symbol '" + node->name + "'.");
    }

    // Ensure the initializer is a literal
    if (node->initial_value->node_type != ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::STRING_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION &&
	    node->initial_value->node_type != ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION &&
	    node->initial_value->node_type != ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION) {
        throw std::runtime_error("Semantic Error: Constant initializer must be a literal value.");
    }

    std::unique_ptr<TypeNode> expr_type = visitExpression(node->initial_value.get());
    if (!areTypesCompatible(expr_type.get(), node->type.get())) {
        throw std::runtime_error("Semantic Error: Type mismatch in constant initialization for '" + node->name + "'.");
    }

    std::unique_ptr<ASTNode> value_clone;
    switch (node->initial_value->node_type) {
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION:
            value_clone = std::make_unique<IntegerLiteralExpressionNode>(static_cast<IntegerLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
            value_clone = std::make_unique<StringLiteralExpressionNode>(static_cast<StringLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
            value_clone = std::make_unique<BooleanLiteralExpressionNode>(static_cast<BooleanLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            value_clone = std::make_unique<CharacterLiteralExpressionNode>(static_cast<CharacterLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION:
            value_clone = std::make_unique<FloatLiteralExpressionNode>(static_cast<FloatLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION:
            value_clone = std::make_unique<DoubleLiteralExpressionNode>(static_cast<DoubleLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        default:
            // Should not happen due to the check above
            break;
    }

    Symbol symbol(Symbol::SymbolType::CONSTANT, node->name, node->type->clone(), std::move(value_clone));
    node->resolved_symbol = symbolTable.addSymbol(std::move(symbol));
}

void SemanticAnalyzer::visit(EnumStatementNode* node) {
    if (symbolTable.current_scope->lookup(node->name)) {
        throw std::runtime_error("Semantic Error: Redefinition of symbol '" + node->name + "'.");
    }

    auto enum_info = std::make_shared<EnumInfo>();
    enum_info->name = node->name;

    symbolTable.addSymbol(Symbol(Symbol::SymbolType::ENUM_TYPE, node->name, enum_info));

    int current_value = 0;
    for (const auto& member : node->members) {
        if (symbolTable.current_scope->lookup(member->name)) {
            throw std::runtime_error("Semantic Error: Redefinition of symbol '" + member->name + "'.");
        }

        if (member->value) {
            // TODO: for now, we only support integer literals as enum values
            if (member->value->node_type != ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION) {
                throw std::runtime_error("Semantic Error: Enum member value must be an integer literal.");
            }
            current_value = static_cast<IntegerLiteralExpressionNode*>(member->value.get())->value;
        }

	auto value_node = std::make_unique<IntegerLiteralExpressionNode>(current_value);
        auto type_node = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
        symbolTable.addSymbol(Symbol(Symbol::SymbolType::CONSTANT, member->name, std::move(type_node), std::move(value_node)));

        current_value++;
    }
}


std::unique_ptr<TypeNode> SemanticAnalyzer::visitExpression(ASTNode* expr) {
    if (!expr) {
        throw std::runtime_error("Semantic Error: Attempted to visit a null expression.");
    }

    std::unique_ptr<TypeNode> result_type = nullptr;

    switch (expr->node_type) {
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION: { 
            result_type = visitIntegerLiteralExpression(static_cast<IntegerLiteralExpressionNode*>(expr));
            break;
        } case ASTNode::NodeType::STRING_LITERAL_EXPRESSION: {
            result_type = visitStringLiteralExpression(static_cast<StringLiteralExpressionNode*>(expr));
            break;
        } case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION: {
            result_type = visitBooleanLiteralExpression(static_cast<BooleanLiteralExpressionNode*>(expr));
            break;
        } case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION: {
            result_type = visitCharacterLiteralExpression(static_cast<CharacterLiteralExpressionNode*>(expr));
            break;
        } case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION: {
            result_type = visitFloatLiteralExpression(static_cast<FloatLiteralExpressionNode*>(expr));
            break;
        } case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION: {
            result_type = visitDoubleLiteralExpression(static_cast<DoubleLiteralExpressionNode*>(expr));
            break;
        } case ASTNode::NodeType::VARIABLE_REFERENCE: {
            auto* var_node = static_cast<VariableReferenceNode*>(expr);
            visit(var_node); 
            Symbol* sym = symbolTable.lookup(var_node->name);
            if (!sym || !sym->dataType) throw std::runtime_error("Variable not found or unresolved.");
            result_type = sym->dataType->clone();
            break;
        }
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            auto* bin_node = static_cast<BinaryOperationExpressionNode*>(expr);
            visit(bin_node); 
            if (!bin_node->resolved_type) throw std::runtime_error("Binary op failed type resolution");
            result_type = bin_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::FUNCTION_CALL: {
            auto* func_node = static_cast<FunctionCallNode*>(expr);
            visit(func_node);
            Symbol* func_symbol = symbolTable.lookup(static_cast<FunctionCallNode*>(expr)->function_name);
            if (!func_symbol) {
                throw std::runtime_error("Semantic Error: Function '" + static_cast<FunctionCallNode*>(expr)->function_name + "' not found.");
            }
            func_node->resolved_type = func_symbol->dataType->clone(); 
            result_type = func_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION: {
            auto* member_node = static_cast<MemberAccessNode*>(expr);
            visit(static_cast<MemberAccessNode*>(expr));
            std::unique_ptr<TypeNode> base_type = visitExpression(static_cast<MemberAccessNode*>(expr)->struct_expr.get());
            const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(base_type.get());
            if (!symbolTable.isStructDefined(struct_type->struct_name)) {
                throw std::runtime_error("Semantic Error: Undefined struct '" + struct_type->struct_name + "'.");
            }
            const auto& struct_def = symbolTable.getStructDefinitions()[struct_type->struct_name];
            for (const auto& member : struct_def->members) {
                if (member.name == static_cast<MemberAccessNode*>(expr)->member_name) {
                    expr->resolved_type = member.type->clone();
                    member_node->resolved_type = member.type->clone();
                    result_type = member.type->clone();
                    break;
                }
            }
            throw std::runtime_error("Semantic Error: Member '" + static_cast<MemberAccessNode*>(expr)->member_name + "' not found in struct '" + struct_type->struct_name + "'.");
        }
        case ASTNode::NodeType::UNARY_OP_EXPRESSION: {
            auto* unary_node = static_cast<UnaryOpExpressionNode*>(expr);
            visit(unary_node);
            if (!unary_node->resolved_type) throw std::runtime_error("Unary op failed type resolution");
            result_type = unary_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION: {
            visit(static_cast<ArrayAccessNode*>(expr));
            std::unique_ptr<TypeNode> array_type = visitExpression(static_cast<ArrayAccessNode*>(expr)->array_expr.get());
            const ArrayTypeNode* arr_type = static_cast<const ArrayTypeNode*>(array_type.get());
            expr->resolved_type = arr_type->base_type->clone();
            result_type = arr_type->base_type->clone();
            break;
        }
        case ASTNode::NodeType::VARIABLE_ASSIGNMENT: {
            auto* assign_node = static_cast<VariableAssignmentNode*>(expr);
            visit(assign_node);
            auto left_type = visitExpression(assign_node->left.get());
            assign_node->resolved_type = left_type->clone();
            result_type = left_type->clone();
            break;
        }
        case ASTNode::NodeType::VARIABLE_DECLARATION: {
            auto* decl_node = static_cast<VariableDeclarationNode*>(expr);
            visit(decl_node);
            result_type = decl_node->type->clone();
            break;
        }
        default:
            throw std::runtime_error("Semantic Error: Unexpected AST node type in visitExpression.");
    }

    if (result_type) {
        expr->resolved_type = result_type->clone();
        return result_type;
    }

    throw std::runtime_error("Semantic Error: Expression resolution returned null.");
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitIntegerLiteralExpression(IntegerLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitStringLiteralExpression(StringLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_STRING);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitBooleanLiteralExpression(BooleanLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitCharacterLiteralExpression(CharacterLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_CHAR);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitFloatLiteralExpression(FloatLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_FLOAT);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitDoubleLiteralExpression(DoubleLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_DOUBLE);
    return node->resolved_type->clone();
}

std::string SemanticAnalyzer::typeToString(const TypeNode* type) {
    if (!type) return "null";
    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            auto p = static_cast<const PrimitiveTypeNode*>(type);
            switch (p->primitive_type) {
                case Token::KEYWORD_INT:    return "int";
                case Token::KEYWORD_FLOAT:  return "float";
                case Token::KEYWORD_DOUBLE: return "double";
                case Token::KEYWORD_BOOL:   return "bool";
                case Token::KEYWORD_CHAR:   return "char";
                case Token::KEYWORD_STRING: return "string";
                case Token::KEYWORD_VOID:   return "void";
                default: return "primitive";
            }
        }
        case TypeNode::TypeCategory::POINTER: return "pointer";
        case TypeNode::TypeCategory::ARRAY:   return "array";
        case TypeNode::TypeCategory::STRUCT: 
            return "struct " + static_cast<const StructTypeNode*>(type)->struct_name;
        default: return "unknown";
    }
}
