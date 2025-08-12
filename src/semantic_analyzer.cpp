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
                case Token::KEYWORD_INT: return 8; // 8 bytes for int (qword)
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
            int element_size = getTypeSize(array_type->base_type.get());
            if (array_type->size > 0) {
                return element_size * array_type->size;
            }
            // For unsized arrays, we might return 0 or throw an error depending on context
            // For now, let's assume fixed-size arrays or handle dynamically later.
            return 0; // Or throw error for unsized array in this context
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
        // Special case: int and bool might be compatible in some contexts (e.g., 0/1 for false/true)
        // For now, let's keep it strict.
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
            // Recursively check base types
            return areTypesCompatible(ptr1->base_type.get(), ptr2->base_type.get());
        }
        case TypeNode::TypeCategory::ARRAY: {
            const ArrayTypeNode* arr1 = static_cast<const ArrayTypeNode*>(type1);
            const ArrayTypeNode* arr2 = static_cast<const ArrayTypeNode*>(type2);
            // Recursively check base types. Array sizes don't affect compatibility for assignment.
            return areTypesCompatible(arr1->base_type.get(), arr2->base_type.get());
        }
        case TypeNode::TypeCategory::STRUCT: {
            const StructTypeNode* s1 = static_cast<const StructTypeNode*>(type1);
            const StructTypeNode* s2 = static_cast<const StructTypeNode*>(type2);
            // Structs are compatible if they have the same name (nominal typing)
            return s1->struct_name == s2->struct_name;
        }
        default:
            return false; // Unknown category
    }
}

void SemanticAnalyzer::analyze() {
    // Start with the global scope
    symbolTable.enterScope();

    // First pass: Process struct definitions to populate their sizes and member offsets
    // ... (existing code for struct processing)

    // Second pass: Add function declarations to the global scope
    for (const auto& func_node : program_ast->functions) {
        // Create a TypeNode for the return type
        std::unique_ptr<TypeNode> return_type;
        if (func_node->return_type_token == Token::KEYWORD_INT) {
            return_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
        } else if (func_node->return_type_token == Token::KEYWORD_VOID) {
            return_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_VOID);
        }
        // Add more cases for other return types (string, bool, char, structs)
        // For simplicity, assuming only int and void for now.

        Symbol func_symbol(Symbol::SymbolType::FUNCTION, std::string(func_node->name), static_cast<std::unique_ptr<TypeNode>>(std::move(return_type)));
        symbolTable.addSymbol(std::move(func_symbol));
    }

    // Third pass: Visit all top-level statements and function definitions
    for (const auto& stmt : program_ast->statements) {
        visit(stmt.get());
    }

    for (const auto& func_node : program_ast->functions) {
        visit(func_node.get());
    }

    // Exit global scope
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
        // Literal expressions are handled by visitExpression
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION:
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            // These are typically visited as part of an expression, not directly as statements
            break;
        default:
            throw std::runtime_error("Semantic Error: Unknown AST node type encountered during analysis.");
    }
}

void SemanticAnalyzer::visit(ProgramNode* node) {
    // Top-level analysis is handled in analyze() method
    // This visitor might be used for recursive calls if needed
}

void SemanticAnalyzer::visit(FunctionDefinitionNode* node) {
    symbolTable.enterScope(); // Enter function scope

    // Add parameters to symbol table with calculated offsets
    int param_offset = 16; // Standard x64 calling convention (return address + old rbp)
    for (const auto& param : node->parameters) {
        int size = getTypeSize(param->type.get());
        symbolTable.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, param->name, param->type->clone(), param_offset, size));
        param_offset += size; // Increment offset by parameter size
    }

    // Initialize currentOffset for local variables in this scope
    symbolTable.scopes.back()->currentOffset = 0; // Local variables start from 0 (negative offsets from rbp)

    // Visit function body statements
    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    symbolTable.exitScope(); // Exit function scope
}

void SemanticAnalyzer::visit(VariableDeclarationNode* node) {
    // Check if variable is already declared in current scope
    if (symbolTable.scopes.back()->lookup(node->name)) {
        throw std::runtime_error("Semantic Error: Redefinition of variable '" + node->name + "'.");
    }

    int var_size = getTypeSize(node->type.get());

    // Assign offset for local variable (relative to rbp)
    // Offsets are negative from rbp for local variables
    symbolTable.scopes.back()->currentOffset -= var_size;
    int offset = symbolTable.scopes.back()->currentOffset;

    Symbol symbol(Symbol::SymbolType::VARIABLE, node->name, node->type->clone(), offset, var_size);
    node->resolved_symbol = symbolTable.addSymbol(std::move(symbol));

    if (node->initial_value) {
        std::unique_ptr<TypeNode> expr_type = visitExpression(node->initial_value.get());
        // Type check initial value
        if (!areTypesCompatible(expr_type.get(), node->type.get())) {
            throw std::runtime_error("Semantic Error: Type mismatch in variable initialization for '" + node->name + "'.");
        }
    }
}

void SemanticAnalyzer::visit(VariableAssignmentNode* node) {
    Symbol* var_symbol = symbolTable.lookup(node->name);
    if (!var_symbol) {
        throw std::runtime_error("Semantic Error: Assignment to undeclared variable '" + node->name + "'.");
    }
    node->resolved_symbol = var_symbol; // Store the resolved symbol

    std::unique_ptr<TypeNode> expr_type = visitExpression(node->expression.get());

    if (node->index_expression) {
        // Array assignment
        std::unique_ptr<TypeNode> index_type = visitExpression(node->index_expression.get());
        if (index_type->category != TypeNode::TypeCategory::PRIMITIVE ||
            static_cast<PrimitiveTypeNode*>(index_type.get())->primitive_type != Token::KEYWORD_INT) {
            throw std::runtime_error("Semantic Error: Array index must be an integer.");
        }

        // Get the base type of the array
        if (var_symbol->dataType->category != TypeNode::TypeCategory::ARRAY) {
            throw std::runtime_error("Semantic Error: Attempted to index a non-array variable '" + node->name + "'.");
        }
        const ArrayTypeNode* array_type = static_cast<const ArrayTypeNode*>(var_symbol->dataType.get());

        // Compare the expression type with the array's base type
        if (!areTypesCompatible(expr_type.get(), array_type->base_type.get())) {
            throw std::runtime_error("Semantic Error: Type mismatch in array element assignment for '" + node->name + "'.");
        }
    } else {
        // Simple variable assignment
        if (!areTypesCompatible(expr_type.get(), var_symbol->dataType.get())) {
            throw std::runtime_error("Semantic Error: Type mismatch in assignment for '" + node->name + "'.");
        }
    }
}

void SemanticAnalyzer::visit(VariableReferenceNode* node) {
    Symbol* var_symbol = symbolTable.lookup(node->name);
    if (!var_symbol) {
        throw std::runtime_error("Semantic Error: Use of undeclared variable '" + node->name + "'.");
    }
    node->resolved_symbol = var_symbol; // Store the resolved symbol
}

void SemanticAnalyzer::visit(BinaryOperationExpressionNode* node) {
    std::unique_ptr<TypeNode> left_type = visitExpression(node->left.get());
    std::unique_ptr<TypeNode> right_type = visitExpression(node->right.get());

    // Basic type compatibility check for binary operations
    if (left_type->category != right_type->category) {
        throw std::runtime_error("Semantic Error: Type mismatch in binary operation.");
    }
    // More detailed checks based on operator (e.g., can't add strings directly)
}

void SemanticAnalyzer::visit(PrintStatementNode* node) {
    visitExpression(node->expression.get());
}

void SemanticAnalyzer::visit(ReturnStatementNode* node) {
    // For now, just visit the expression.
    // In a full compiler, would check return type against function's declared return type.
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

    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }
}

void SemanticAnalyzer::visit(ForStatementNode* node) {
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
}

void SemanticAnalyzer::visit(FunctionCallNode* node) {
    // For now, just visit arguments.
    // In a full compiler, would look up function in symbol table, check arity and argument types.
    Symbol* func_symbol = symbolTable.lookup(node->function_name);
    if (!func_symbol || func_symbol->type != Symbol::SymbolType::FUNCTION) {
        throw std::runtime_error("Semantic Error: Call to undeclared function '" + node->function_name + "'.");
    }
    node->resolved_symbol = func_symbol; // Store the resolved symbol

    for (const auto& arg : node->arguments) {
        visitExpression(arg.get());
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

    // Check if member exists in the struct
    bool member_found = false;
    for (const auto& member : struct_def->members) {
        if (member.name == node->member_name) {
            member_found = true;
            // Create a temporary Symbol for the member and store it
            // The offset is already stored in member.offset
            node->resolved_symbol = new Symbol(Symbol::SymbolType::STRUCT_MEMBER, member.name, member.type->clone(), member.offset, getTypeSize(member.type.get()));
            // The created symbol is owned by the AST node, so it will be deleted when the AST node is deleted.
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
        // Operand must be a variable reference
        if (node->operand->node_type != ASTNode::NodeType::VARIABLE_REFERENCE) {
            throw std::runtime_error("Semantic Error: Address-of operator '&' can only be applied to variables.");
        }
        // Store the resolved symbol (the variable itself)
        node->resolved_symbol = static_cast<VariableReferenceNode*>(node->operand.get())->resolved_symbol;
    } else if (node->op_type == Token::STAR) {
        // Operand must be a pointer type
        if (operand_type->category != TypeNode::TypeCategory::POINTER) {
            throw std::runtime_error("Semantic Error: Dereference operator '*' can only be applied to pointer types.");
        }
        // The resolved symbol is the base type of the pointer (dereferenced value)
        node->resolved_symbol = new Symbol(Symbol::SymbolType::VARIABLE, "", static_cast<PointerTypeNode*>(operand_type.get())->base_type->clone(), 0, getTypeSize(static_cast<PointerTypeNode*>(operand_type.get())->base_type.get()));
        // The created symbol is owned by the AST node, so it will be deleted when the AST node is deleted.
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
    // The result type is the base type of the array
    node->resolved_symbol = new Symbol(Symbol::SymbolType::VARIABLE, "", static_cast<ArrayTypeNode*>(array_type.get())->base_type->clone(), 0, getTypeSize(static_cast<ArrayTypeNode*>(array_type.get())->base_type.get()));
    // The created symbol is owned by the AST node, so it will be deleted when the AST node is deleted.
}

void SemanticAnalyzer::visit(StructDefinitionNode* node) {
    // This visitor is primarily for calculating member offsets and total size
    // The struct definition itself is already in the symbol table from parsing phase.
    // We need to update the stored StructDefinitionNode in the symbol table with size/offsets.

    // For now, the getTypeSize(StructTypeNode) handles the size calculation.
    // Member offsets will be handled during code generation or by enhancing the Symbol.
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
            visit(static_cast<VariableReferenceNode*>(expr)); // Perform symbol lookup
            Symbol* sym = symbolTable.lookup(static_cast<VariableReferenceNode*>(expr)->name);
            if (!sym) throw std::runtime_error("Semantic Error: Variable '" + static_cast<VariableReferenceNode*>(expr)->name + "' not found.");
            return sym->dataType->clone();
        }
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            visit(static_cast<BinaryOperationExpressionNode*>(expr));
            // For simplicity, assume result type is same as operand types for now
            return visitExpression(static_cast<BinaryOperationExpressionNode*>(expr)->left.get());
        }
        case ASTNode::NodeType::FUNCTION_CALL: {
            visit(static_cast<FunctionCallNode*>(expr));
            // For now, assume int return type for function calls.
            // In a full compiler, would look up function's return type from symbol table.
            return std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
        }
        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION: {
            visit(static_cast<MemberAccessNode*>(expr));
            // Need to return the type of the accessed member
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
            // Return the appropriate type based on the unary operation
            const UnaryOpExpressionNode* unary_node = static_cast<const UnaryOpExpressionNode*>(expr);
            std::unique_ptr<TypeNode> operand_type = visitExpression(unary_node->operand.get());
            if (unary_node->op_type == Token::ADDRESSOF) {
                return std::make_unique<PointerTypeNode>(std::move(operand_type));
            } else if (unary_node->op_type == Token::STAR) {
                const PointerTypeNode* ptr_type = static_cast<const PointerTypeNode*>(operand_type.get());
                return ptr_type->base_type->clone();
            }
            throw std::runtime_error("Semantic Error: Unhandled unary operator type in visitExpression.");
        }
        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION: {
            visit(static_cast<ArrayAccessNode*>(expr));
            std::unique_ptr<TypeNode> array_type = visitExpression(static_cast<ArrayAccessNode*>(expr)->array_expr.get());
            const ArrayTypeNode* arr_type = static_cast<const ArrayTypeNode*>(array_type.get());
            return arr_type->base_type->clone();
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
