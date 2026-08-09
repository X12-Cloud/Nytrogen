#include "semantic_analyzer.hpp"

#include <cstring>
#include <iostream>
#include <set>
#include <stdexcept>

// Helper to get size of a type
int SemanticAnalyzer::getTypeSize(const TypeNode* type) {
    if (type == nullptr) {
        Logger::report_error("Semantic Error", "Attempted to get size of a null type.");
    }

    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const PrimitiveTypeNode* prim_type = static_cast<const PrimitiveTypeNode*>(type);
            switch (prim_type->primitive_type) {
                case Token::KEYWORD_INT:  // 4 bytes for int and float (dword)
                case Token::KEYWORD_FLOAT:
                    return 4;
                case Token::KEYWORD_DOUBLE:
                    return 8;              // 8 bytes for double (qword)
                case Token::KEYWORD_BOOL:  // 1 byte for bool and char
                case Token::KEYWORD_CHAR:
                    return 1;
                case Token::KEYWORD_STRING:
                    return 8;  // 8 bytes for string (pointer)
                case Token::KEYWORD_VOID:
                    return 0;  // Void has no size
                case Token::KEYWORD_COMPLEX:
                    return 16; // 2 doubles
                case Token::KEYWORD_MATRIX:
                    return 64; // 4 complex numbers
                case Token::KEYWORD_QUBIT:
                    return 8; // 8 byte offset
                default:
                    Logger::report_error("Semantic Error",
                                         "Unknown primitive type for size calculation.");
            }
        }
        case TypeNode::TypeCategory::POINTER:
            return 8;  // Pointers are 8 bytes on x64
        case TypeNode::TypeCategory::ARRAY: {
            const ArrayTypeNode* array_type = static_cast<const ArrayTypeNode*>(type);
            if (array_type->size <= 0) {
                Logger::report_error("Semantic Error",
                                     "Unsized arrays not allowed for local variables.");
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
                Logger::report_error("Semantic Error",
                                     "Undefined struct '" + struct_type->struct_name + "'.");
            }
            return symbolTable.getStructDefinitions()[struct_type->struct_name]->size;
        }
        default:
            Logger::report_error("Semantic Error", "Unknown type category for size calculation.");
    }
}

bool SemanticAnalyzer::areTypesCompatible(const TypeNode* type1, const TypeNode* type2) {
    if ((type1 == nullptr) || (type2 == nullptr)) {
        return false;  // Null types are not compatible
    }

    auto isString = [](const TypeNode* t) {
        if (t->category != TypeNode::TypeCategory::PRIMITIVE) {
            return false;
        }
        auto prim = static_cast<const PrimitiveTypeNode*>(t);
        return prim->primitive_type == Token::KEYWORD_STRING;
    };

    bool t1_is_ptr = (type1->category == TypeNode::TypeCategory::POINTER ||
                      type1->category == TypeNode::TypeCategory::ARRAY);
    bool t2_is_ptr = (type2->category == TypeNode::TypeCategory::POINTER ||
                      type2->category == TypeNode::TypeCategory::ARRAY);

    if ((t1_is_ptr && isString(type2)) || (t2_is_ptr && isString(type1))) {
        return true;
    }

    if (type1->category != type2->category) {
        bool isPointer = (type1->category == TypeNode::TypeCategory::POINTER ||
                          type1->category == TypeNode::TypeCategory::ARRAY);
        bool isOtherPointer = (type2->category == TypeNode::TypeCategory::POINTER ||
                               type2->category == TypeNode::TypeCategory::ARRAY);

        if (!(isPointer && isOtherPointer)) {
            return false;
        }
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
            return false;  // Unknown category
    }
}

void SemanticAnalyzer::analyze() {
    symbolTable.enterScope();  // global scope

    // Process structs
    for (const auto& struct_node : program_ast->structs) {
        visit(struct_node.get());
    }

    // Bootstrap built in gates
    std::vector<std::string> built_ins = {"h", "x", "z", "s", "t"};
    for (const auto& name : built_ins) {
        Symbol gate_sym(Symbol::SymbolType::VARIABLE, name, 
                        std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_MATRIX));
        gate_sym.is_global = true;
        gate_sym.mangled_name = name;
        symbolTable.addSymbol(std::move(gate_sym));
    }

    // Declare functions (but don't visit bodies yet)
    for (const auto& func_node : program_ast->functions) {
        std::unique_ptr<TypeNode> return_type = func_node->return_type->clone();
        std::vector<std::unique_ptr<TypeNode>> param_types;
        for (const auto& param : func_node->parameters) {
            param_types.push_back(param->type->clone());
        }

        std::string mangled = Mangler::mangleFunction(namespace_stack, func_node->name);

        Symbol func_symbol(Symbol::SymbolType::FUNCTION, std::string(func_node->name),
                           std::move(return_type), std::move(param_types));
        func_symbol.mangled_name = mangled;
        symbolTable.addSymbol(std::move(func_symbol));
    }

    // Process global statements
    for (const auto& stmt : program_ast->statements) {
        visit(stmt.get());
    }

    for (const auto& func_node : program_ast->functions) {
        visit(func_node.get());
    }

    // Check for main
    bool has_main = false;
    for (const auto& func : program_ast->functions) {
        if (func->name == "main") {
            has_main = true;
            if (func->return_type->category != TypeNode::TypeCategory::PRIMITIVE ||
                static_cast<PrimitiveTypeNode*>(func->return_type.get())->primitive_type !=
                    Token::KEYWORD_INT) {
                Logger::report_error("Semantic Error", "'main' function must return int.");
            }
            if (!func->parameters.empty()) {
                Logger::report_error("Semantic Error",
                                     "'main' function should have no parameters.");
            }
        }
    }
    if (is_entry_point && !has_main) {
        Logger::report_error("Semantic Error", "No 'main' function defined.");
    }
}

void SemanticAnalyzer::visit(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

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
        case ASTNode::NodeType::NAMESPACE_DEFINITION:
            visit(static_cast<NamespaceDefinition*>(node));
            break;
        case ASTNode::NodeType::SCOPE_RESOLUTION:
            visit(static_cast<ScopeResolutionNode*>(node));
            break;
        case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION: {
            auto* lit = static_cast<FloatLiteralExpressionNode*>(node);
            lit->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_FLOAT);
            break;
        }
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION: {
            auto* lit = static_cast<DoubleLiteralExpressionNode*>(node);
            lit->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_DOUBLE);
            break;
        }
        case ASTNode::NodeType::COMPLEX_LITERAL_EXPRESSION: {
            auto* lit = static_cast<ComplexLiteralExpressionNode*>(node);
            lit->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_COMPLEX);
            break;
        }
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION: {
            auto* lit = static_cast<IntegerLiteralExpressionNode*>(node);
            lit->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
            break;
        }
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
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
        case ASTNode::NodeType::QUBIT_DEFINITION:
            visit(static_cast<QubitDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::GATE_APPLICATION_OPERATION_EXPRESSION:
            visit(static_cast<GateAppOperationExpressionNode*>(node));
            break;
        default:
            Logger::report_error("Semantic Error",
                                 "Unknown AST node type encountered during analysis.");
    }
}

void SemanticAnalyzer::visit(ProgramNode* node) {}

void SemanticAnalyzer::visit(FunctionDefinitionNode* node) {
    std::vector<std::unique_ptr<TypeNode>> paramTypes;
    for (const auto& param : node->parameters) {
        paramTypes.push_back(param->type->clone());
    }

    Symbol func_symbol(Symbol::SymbolType::FUNCTION, node->name, node->return_type->clone(),
                       std::move(paramTypes));

    func_symbol.mangled_name = Mangler::mangleFunction(namespace_stack, node->name);
    node->mangled_name = func_symbol.mangled_name;

    symbolTable.addSymbol(std::move(func_symbol));

    symbolTable.enterScope();
    symbolTable.current_scope->scope_name = node->mangled_name;

    currentFunctionReturnType = nullptr;

    const std::vector<std::string> arg_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int param_offset = 16;
    int register_param_offset = 0;

    for (int i = 0; i < node->parameters.size(); ++i) {
        const auto& param = node->parameters[i];
        int size = getTypeSize(param->type.get());
        Symbol param_sym(Symbol::SymbolType::VARIABLE, param->name, param->type->clone(), param_offset, size);
        param_sym.is_global = false;
        param_sym.mangled_name = "";
        if (i < arg_registers.size()) {
            register_param_offset -= 8;
            symbolTable.addSymbol(Symbol(Symbol::SymbolType::VARIABLE, param->name,
                                         param->type->clone(), register_param_offset, size));
        } else {
            symbolTable.addSymbol(std::move(param_sym));
            param_offset += size;
        }
    }

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
    bool global_context = (symbolTable.current_scope->parent == nullptr);
    bool is_global = (symbolTable.all_scopes.size() <= 2) || global_context;

    for (auto& decl : node->declarations) {
        if (symbolTable.current_scope->lookup(decl.name) != nullptr) {
            Logger::report_error("Semantic Error", "Redefinition of variable '" + decl.name + "'.");
        }

        std::unique_ptr<TypeNode> actual_type;

        if (is_auto) {
            if (!decl.initial_value) {
                Logger::report_error("Semantic Error",
                                     "'auto' variable '" + decl.name + "' requires an initializer.",
                                     node->line);
            }
            actual_type = visitExpression(decl.initial_value.get());
            if (!actual_type) {
                Logger::report_error(
                    "Semantic Error",
                    "Could not deduce type for 'auto' variable '" + decl.name + "'.", node->line);
            }
        } else {
            actual_type = node->type->clone();
            if (decl.initial_value) {
                auto expr_type = visitExpression(decl.initial_value.get());
                if (!areTypesCompatible(expr_type.get(), actual_type.get())) {
                    Logger::report_error("Semantic Error",
                                         "Type mismatch for initialization of '" + decl.name + "'.",
                                         node->line);
                }
            }
        }

        if (!actual_type) {
            Logger::report_error("Semantic Error", "Type deduction failed for '" + decl.name + "'.",
                                 node->line);
        }

        int var_size = getTypeSize(actual_type.get());
        symbolTable.current_scope->currentOffset -= var_size;
        int offset = symbolTable.current_scope->currentOffset;

        std::string unique_label = Mangler::mangleVariable(namespace_stack, decl.name);
        Symbol symbol(Symbol::SymbolType::VARIABLE, decl.name, actual_type->clone(), offset,
                      var_size);

        symbol.mangled_name = unique_label;
        symbol.is_global = is_global;

        decl.resolved_symbol = symbolTable.addSymbol(std::move(symbol));
    }
}

void SemanticAnalyzer::visit(VariableAssignmentNode* node) {
    if (node->left->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
        auto* var_ref = static_cast<VariableReferenceNode*>(node->left.get());
        Symbol* symbol = symbolTable.lookup(var_ref->name);
        if ((symbol != nullptr) && symbol->type == Symbol::SymbolType::CONSTANT) {
            Logger::report_error("Semantic Error",
                                 "Cannot assign to constant '" + var_ref->name + "'.", node->line);
        }
    }

    std::unique_ptr<TypeNode> left_type = visitExpression(node->left.get());
    std::unique_ptr<TypeNode> right_type = visitExpression(node->right.get());

    if (!areTypesCompatible(left_type.get(), right_type.get())) {
        Logger::report_error("Semantic Error", "Type mismatch in assignment.", node->line);
    }
}

void SemanticAnalyzer::visit(VariableReferenceNode* node) {
    Symbol* var_symbol = symbolTable.lookup(node->name);
    if (var_symbol == nullptr) {
        Logger::report_error("Semantic Error", "Use of undeclared variable '" + node->name + "'.",
                             node->line);
    }
    node->resolved_symbol = var_symbol;
    node->resolved_offset = var_symbol->offset;
    node->resolved_type = var_symbol->dataType->clone();
}

void SemanticAnalyzer::visit(NamespaceDefinition* node) {
    namespace_stack.push_back(node->name);

    Symbol* existing_ns = symbolTable.current_scope->lookup(node->name);

    if (existing_ns != nullptr && existing_ns->type == Symbol::SymbolType::NAMESPACE_DEFINITION) {
        Scope* backup_scope = symbolTable.current_scope;
        symbolTable.current_scope = existing_ns->internal_scope;

        for (auto& member : node->members) {
            this->visit(member.node.get());
        }
        symbolTable.current_scope = backup_scope;
    } else {
        symbolTable.enterScope();

        Scope* namespace_scope = symbolTable.current_scope;
        Symbol ns_symbol(Symbol::SymbolType::NAMESPACE_DEFINITION, node->name, namespace_scope);
        if (namespace_scope->parent != nullptr) {
            namespace_scope->parent->addSymbol(std::move(ns_symbol));
        }

        for (auto& member : node->members) {
            this->visit(member.node.get());
        }
        symbolTable.exitScope();
    }

    namespace_stack.pop_back();
}

void SemanticAnalyzer::visit(ScopeResolutionNode* node) {
    Symbol* ns_symbol = symbolTable.lookup(node->namespace_name);
    if ((ns_symbol == nullptr) || ns_symbol->type != Symbol::SymbolType::NAMESPACE_DEFINITION) {
        Logger::report_error("Semantic Error", "'" + node->namespace_name + "' is not a namespace.",
                             node->line);
    }

    Scope* old_scope = symbolTable.current_scope;

    bool is_top_level_namespace = (this->original_context == nullptr);
    if (is_top_level_namespace) {
        this->original_context = old_scope;
    }
    symbolTable.current_scope = ns_symbol->internal_scope;

    try {
        this->visit(node->member.get());

        if (node->member->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
            auto* var = static_cast<VariableReferenceNode*>(node->member.get());
            node->resolved_type = var->resolved_type->clone();
        } else if (node->member->node_type == ASTNode::NodeType::SCOPE_RESOLUTION) {
            auto* nested = static_cast<ScopeResolutionNode*>(node->member.get());
            if (nested->resolved_type) {
                node->resolved_type = nested->resolved_type->clone();
            }
        } else if (node->member->node_type == ASTNode::NodeType::FUNCTION_CALL) {
            auto* call = static_cast<FunctionCallNode*>(node->member.get());
            if ((call->resolved_symbol != nullptr) && call->resolved_symbol->dataType) {
                node->resolved_type = call->resolved_symbol->dataType->clone();
            }
        }
        if (!node->resolved_type) {
            Logger::report_error(
                "Semantic Error",
                "Could not resolve type for namespace member '" + node->namespace_name + "'.",
                node->line);
        }

    } catch (const std::exception& e) {
        symbolTable.current_scope = old_scope;
        throw;
    }

    if (is_top_level_namespace) {
        this->original_context = nullptr;
    }
    symbolTable.current_scope = old_scope;
}

void SemanticAnalyzer::visit(BinaryOperationExpressionNode* node) {
    std::unique_ptr<TypeNode> left_type = visitExpression(node->left.get());
    std::unique_ptr<TypeNode> right_type = visitExpression(node->right.get());

    node->left->resolved_type = left_type->clone();
    node->right->resolved_type = right_type->clone();

    if (left_type->category != right_type->category) {
        Logger::report_error("Semantic Error",
                             "Type mismatch in binary operation (cannot operate on " +
                                 typeToString(left_type.get()) + " and " +
                                 typeToString(right_type.get()) + ")",
                             node->line);
    }

    if (left_type->category == TypeNode::TypeCategory::PRIMITIVE) {
        auto p1 = static_cast<PrimitiveTypeNode*>(left_type.get());
        auto p2 = static_cast<PrimitiveTypeNode*>(right_type.get());
        if (p1->primitive_type != p2->primitive_type) {
            Logger::report_error(
                "Semantic Error",
                "Mixed math. Adding different primitive types is not yet supported.", node->line);
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
            if (left_type) {
                std::cout << "Debug: Binary Op resolving to: " << typeToString(left_type.get())
                          << std::endl;
            }
            node->resolved_type = left_type->clone();
            break;
    }
}

void SemanticAnalyzer::visit(GateAppOperationExpressionNode* node) {
    std::unique_ptr<TypeNode> left = visitExpression(node->gate.get());
    std::unique_ptr<TypeNode> right = visitExpression(node->qubit.get());

    node->gate->resolved_type = left->clone();
    node->qubit->resolved_type = right->clone();

    if (node->op_type != Token::ARROW) {
        Logger::report_error("Semantic Error", "Unsupported operator in gate application expression, did you mean `->`.");
    }
}

void SemanticAnalyzer::visit(PrintStatementNode* node) {
    for (const auto& expr : node->expressions) {
        expr->resolved_type = std::move(visitExpression(expr.get()));
        if (!expr->resolved_type) {
            Logger::report_error("Semantic Error", "Could not resolve type for print expression.",
                                 node->line);
        }
    }
}

void SemanticAnalyzer::visit(ReturnStatementNode* node) {
    if (node->expression) {
        node->resolved_type = visitExpression(node->expression.get());

        if (currentFunctionReturnType->category == TypeNode::TypeCategory::PRIMITIVE) {
            auto prim = static_cast<PrimitiveTypeNode*>(currentFunctionReturnType);
            if (prim->primitive_type == Token::KEYWORD_VOID) {
                Logger::report_error("Semantic Error",
                                     "Cannot return a value from a void function.", node->line);
            }
            if (!areTypesCompatible(node->resolved_type.get(), currentFunctionReturnType)) {
                Logger::report_error("Semantic Error", "Return type mismatch in function.",
                                     node->line);
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
            Logger::report_error("Semantic Error", "Non-void function must return a value.",
                                 node->line);
        }
    }
}

void SemanticAnalyzer::visit(IfStatementNode* node) {
    std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
    if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_BOOL &&
            static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type !=
                Token::INTEGER_LITERAL) {
        Logger::report_error("Semantic Error", "If condition must be a boolean expression.",
                             node->line);
    }

    for (const auto& stmt : node->true_block) {
        visit(stmt.get());
    }
    for (const auto& stmt : node->false_block) {
        visit(stmt.get());
    }
}

void SemanticAnalyzer::visit(SwitchStatementNode* node) {
    std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
    if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        dynamic_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_INT) {
        Logger::report_error("Semantic Error", "Switch condition must be an integer value.",
                             node->line);
    }

    std::set<long long> seen_cases;
    bool has_default = false;

    for (auto& case_node : node->cases) {
        if (case_node.is_default) {
            if (has_default) {
                Logger::report_error("Semantic Error", "Multiple 'default' cases found.",
                                     node->line);
            }
            has_default = true;
        } else {
            auto* literal =
                dynamic_cast<IntegerLiteralExpressionNode*>(case_node.constant_expr.get());
            if (literal == nullptr) {
                Logger::report_error("Semantic Error",
                                     "Case label must be a constant integer literal.", node->line);
            }

            if (seen_cases.count(literal->value) != 0u) {
                Logger::report_error(
                    "Semantic Error",
                    "Duplicate case value '" + std::to_string(literal->value) + "'.", node->line);
            }
            seen_cases.insert(literal->value);
        }

        for (auto& stmt : case_node.body) {
            visit(stmt.get());
        }
    }

    if (!seen_cases.empty()) {
        long long min_value = *seen_cases.begin();
        long long max_value = *seen_cases.rbegin();
        long long range = max_value - min_value;

        if (range < 256) {
            node->use_jump_table = true;
            node->min_case = min_value;
            node->max_case = max_value;
        } else {
            node->use_jump_table = false;
        }
    }
}

void SemanticAnalyzer::visit(WhileStatementNode* node) {
    std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
    if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type != Token::KEYWORD_BOOL) {
        Logger::report_error("Semantic Error", "While condition must be a boolean expression.",
                             node->line);
    }

    symbolTable.enterScope();
    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }
    symbolTable.exitScope();
}

void SemanticAnalyzer::visit(ForStatementNode* node) {
    symbolTable.enterScope();  // Scope for initializer, condition, increment, and body

    if (node->initializer) {
        visit(node->initializer.get());
    }
    if (node->condition) {
        std::unique_ptr<TypeNode> cond_type = visitExpression(node->condition.get());
        if (cond_type->category != TypeNode::TypeCategory::PRIMITIVE ||
            static_cast<PrimitiveTypeNode*>(cond_type.get())->primitive_type !=
                Token::KEYWORD_BOOL) {
            Logger::report_error("Semantic Error",
                                 "For loop condition must be a boolean expression.", node->line);
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
    if (node->function_name == "__builtin_sqrt" || node->function_name == "__builtin_abs" ||
        node->function_name == "__builtin_round") {
        if (node->arguments.size() != 1) {
            Logger::report_error("Semantic Error", "__builtin_sqrt/abs/round expect only 1 argument.",
                                 node->line);
        }

        std::unique_ptr<TypeNode> arg_type = visitExpression(node->arguments[0].get());
        node->arguments[0]->resolved_type = arg_type->clone();
        node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_DOUBLE);
        node->resolved_symbol = nullptr;
        return;
    } else if (node->function_name == "exit") {
        if (node->arguments.size() != 1) {
            Logger::report_error("Semantic Error", "exit expect only 1 argument.",
                                 node->line);
        }

        std::unique_ptr<TypeNode> arg_type = visitExpression(node->arguments[0].get());
        node->arguments[0]->resolved_type = arg_type->clone();
        node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
        node->resolved_symbol = nullptr;
        return;
    }

    Symbol* func_symbol = symbolTable.lookup(node->function_name);
    if ((func_symbol == nullptr) || func_symbol->type != Symbol::SymbolType::FUNCTION) {
        Logger::report_error("Semantic Error",
                             "Call to undeclared function '" + node->function_name + "'.",
                             node->line);
    }
    node->resolved_symbol = func_symbol;

    // Check number of arguments
    if (node->arguments.size() != func_symbol->parameterTypes.size()) {
        Logger::report_error("Semantic Error",
                             "Function '" + node->function_name + "' expects " +
                                 std::to_string(func_symbol->parameterTypes.size()) +
                                 " arguments, but " + std::to_string(node->arguments.size()) +
                                 " were provided.",
                             node->line);
    }

    // Check argument types
    Scope* namespace_scope = symbolTable.current_scope;
    if (this->original_context != nullptr) {
        symbolTable.current_scope = this->original_context;
    }

    for (size_t i = 0; i < node->arguments.size(); ++i) {
        std::unique_ptr<TypeNode> arg_type = visitExpression(node->arguments[i].get());
        node->arguments[i]->resolved_type = arg_type->clone();
        if (!areTypesCompatible(arg_type.get(), func_symbol->parameterTypes[i].get())) {
            Logger::report_error("Semantic Error",
                                 "Type mismatch in argument " + std::to_string(i + 1) +
                                     " of function '" + node->function_name + "'.",
                                 node->line);
        }
    }
    symbolTable.current_scope = namespace_scope;
    if (func_symbol->dataType) {
        node->resolved_type = func_symbol->dataType->clone();
    } else {
        Logger::report_error("Semantic Error",
                             "Function '" + node->function_name + "' has no return type.",
                             node->line);
    }
}

void SemanticAnalyzer::visit(MemberAccessNode* node) {
    if (debug_mode) {
        std::cout << "Debug: Entering visit for node: " << node << std::endl;
    }
    std::unique_ptr<TypeNode> base_type = visitExpression(node->struct_expr.get());

    if (base_type->category != TypeNode::TypeCategory::STRUCT) {
        Logger::report_error("Semantic Error",
                             "Member access operator '.' used on non-struct type.", node->line);
    }

    const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(base_type.get());

    if (!symbolTable.isStructDefined(struct_type->struct_name)) {
        Logger::report_error("Semantic Error",
                             "Undefined struct '" + struct_type->struct_name + "'.", node->line);
    }

    const auto& definitions = symbolTable.getStructDefinitions();
    auto it = definitions.find(struct_type->struct_name);
    if (it == definitions.end()) {
        Logger::report_error("Semantic Error", "Struct not found in registry during access",
                             node->line);
    }
    auto* struct_def = it->second;

    if (debug_mode) {
        std::cout << "Debug: Struct '" << struct_type->struct_name << "' has "
                  << struct_def->members.size() << " members in the registry." << std::endl;
    }
    bool member_found = false;

    for (const auto& member : struct_def->members) {
        if (strcmp(member.name.c_str(), node->member_name.c_str()) == 0) {
            member_found = true;

            // Check visibility
            if (member.visibility == StructMember::Visibility::PRIVATE) {
                // A more complex check would be needed for friend classes or member functions
                Logger::report_error("Semantic Error",
                                     "Cannot access private member '" + node->member_name +
                                         "' of struct '" + struct_type->struct_name + "'.",
                                     node->line);
            }

            node->resolved_symbol =
                new Symbol(Symbol::SymbolType::STRUCT_MEMBER, member.name, member.type->clone(),
                           member.offset, getTypeSize(member.type.get()), member.visibility);
            node->resolved_type = member.type->clone();
            return;
        }
    }

    if (!member_found) {
        Logger::report_error("Semantic Error",
                             "Struct '" + struct_type->struct_name + "' has no member named '" +
                                 node->member_name + "'.",
                             node->line);
    }
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

void SemanticAnalyzer::visit(QubitDefinitionNode* node) {
    bool global_context = (symbolTable.current_scope->parent == nullptr);
    bool is_global = (symbolTable.all_scopes.size() <= 2) || global_context;
    if (symbolTable.current_scope->lookup(node->qubit_name) != nullptr) {
        Logger::report_error("Semantic Error", "Redefinition of qubit '" + node->qubit_name + "'.", node->line);
    }

    if (node->has_custom_amplitudes) {
        visit(node->alpha.get());
        visit(node->beta.get());
    }

    int stack_offset = 0;
    if (!is_global) {
        symbolTable.current_scope->currentOffset -= 8;
        stack_offset = symbolTable.current_scope->currentOffset;
    }

    Symbol q_sym(Symbol::SymbolType::VARIABLE, node->qubit_name, 
                 std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_QUBIT), 
                 stack_offset, 8);

    std::string unique_label = Mangler::mangleVariable(namespace_stack, node->qubit_name);

    q_sym.mangled_name = unique_label;
    q_sym.is_global = is_global;

    symbolTable.addSymbol(std::move(q_sym));
}

void SemanticAnalyzer::visit(UnaryOpExpressionNode* node) {
    std::unique_ptr<TypeNode> operand_type = visitExpression(node->operand.get());

    if (Utils::isTypeKeyword(node->op_type)) {
        node->resolved_type = std::make_unique<PrimitiveTypeNode>(node->op_type);

        // Optional: Add cast validation here
        // e.g., if (!canCast(operand_type.get(), node->resolved_type.get())) Logger::report_error("Invalid cast...", node->line);
        return;
    }

    if (node->op_type == Token::ADDRESSOF) {
        if (node->operand->node_type != ASTNode::NodeType::VARIABLE_REFERENCE) {
            Logger::report_error("Semantic Error",
                                 "Address-of operator '&' can only be applied to variables.",
                                 node->line);
        }
        node->resolved_symbol =
            static_cast<VariableReferenceNode*>(node->operand.get())->resolved_symbol;
        node->resolved_type = std::make_unique<PointerTypeNode>(std::move(operand_type));
    } else if (node->op_type == Token::STAR) {
        if (operand_type->category != TypeNode::TypeCategory::POINTER) {
            Logger::report_error("Semantic Error",
                                 "Dereference operator '*' can only be applied to pointer types.",
                                 node->line);
        }
        node->resolved_type = static_cast<PointerTypeNode*>(operand_type.get())->base_type->clone();
    } else if (node->op_type == Token::BANG) {
        node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::INTEGER_LITERAL);
    } else if (node->op_type == Token::MINUS) {
        if (operand_type) {
            node->resolved_type = operand_type->clone();
        } else {
            Logger::report_error("Semantic Error", "Invalid operand type for negation.",
                                 node->line);
        }
    } else {
        Logger::report_error("Semantic Error", "Unknown unary operator.", node->line);
    }
}

void SemanticAnalyzer::visit(ArrayAccessNode* node) {
    std::unique_ptr<TypeNode> array_type = visitExpression(node->array_expr.get());
    std::unique_ptr<TypeNode> index_type = visitExpression(node->index_expr.get());

    if (array_type->category != TypeNode::TypeCategory::ARRAY) {
        Logger::report_error("Semantic Error", "Array access operator '[]' used on non-array type.",
                             node->line);
    }

    if (index_type->category != TypeNode::TypeCategory::PRIMITIVE ||
        static_cast<PrimitiveTypeNode*>(index_type.get())->primitive_type != Token::KEYWORD_INT) {
        Logger::report_error("Semantic Error", "Array index must be an integer.", node->line);
    }
    node->resolved_type = static_cast<ArrayTypeNode*>(array_type.get())->base_type->clone();
}

void SemanticAnalyzer::visit(AsmStatementNode* node) {
    // No semantic analysis needed for inline assembly
}

void SemanticAnalyzer::visit(ConstantDeclarationNode* node) {
    if (symbolTable.current_scope->lookup(node->name) != nullptr) {
        Logger::report_error("Semantic Error", "Redefinition of symbol '" + node->name + "'.",
                             node->line);
    }

    // Ensure the initializer is a literal
    if (node->initial_value->node_type != ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::STRING_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION &&
        node->initial_value->node_type != ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION) {
        Logger::report_error("Semantic Error", "Constant initializer must be a literal value.",
                             node->line);
    }

    std::unique_ptr<TypeNode> expr_type = visitExpression(node->initial_value.get());
    if (!areTypesCompatible(expr_type.get(), node->type.get())) {
        Logger::report_error("Semantic Error",
                             "Type mismatch in constant initialization for '" + node->name + "'.",
                             node->line);
    }

    std::unique_ptr<ASTNode> value_clone;
    switch (node->initial_value->node_type) {
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION:
            value_clone = std::make_unique<IntegerLiteralExpressionNode>(
                static_cast<IntegerLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
            value_clone = std::make_unique<StringLiteralExpressionNode>(
                static_cast<StringLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
            value_clone = std::make_unique<BooleanLiteralExpressionNode>(
                static_cast<BooleanLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            value_clone = std::make_unique<CharacterLiteralExpressionNode>(
                static_cast<CharacterLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION:
            value_clone = std::make_unique<FloatLiteralExpressionNode>(
                static_cast<FloatLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION:
            value_clone = std::make_unique<DoubleLiteralExpressionNode>(
                static_cast<DoubleLiteralExpressionNode*>(node->initial_value.get())->value);
            break;
        default:
            // Should not happen due to the check above
            break;
    }

    Symbol symbol(Symbol::SymbolType::CONSTANT, node->name, node->type->clone(),
                  std::move(value_clone));
    node->resolved_symbol = symbolTable.addSymbol(std::move(symbol));
}

void SemanticAnalyzer::visit(EnumStatementNode* node) {
    if (symbolTable.current_scope->lookup(node->name) != nullptr) {
        Logger::report_error("Semantic Error", "Redefinition of symbol '" + node->name + "'.",
                             node->line);
    }

    auto enum_info = std::make_shared<EnumInfo>();
    enum_info->name = node->name;

    symbolTable.addSymbol(Symbol(Symbol::SymbolType::ENUM_TYPE, node->name, enum_info));

    int current_value = 0;
    for (const auto& member : node->members) {
        if (symbolTable.current_scope->lookup(member->name) != nullptr) {
            Logger::report_error("Semantic Error", "Redefinition of symbol '" + member->name + "'.",
                                 node->line);
        }

        if (member->value) {
            // TODO: for now, we only support integer literals as enum values
            if (member->value->node_type != ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION) {
                Logger::report_error("Semantic Error",
                                     "Enum member value must be an integer literal.", node->line);
            }
            current_value = static_cast<IntegerLiteralExpressionNode*>(member->value.get())->value;
        }

        auto value_node = std::make_unique<IntegerLiteralExpressionNode>(current_value);
        auto type_node = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
        symbolTable.addSymbol(Symbol(Symbol::SymbolType::CONSTANT, member->name,
                                     std::move(type_node), std::move(value_node)));

        current_value++;
    }
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitExpression(ASTNode* expr) {
    if (expr == nullptr) {
        Logger::report_error("Semantic Error", "Attempted to visit a null expression.");
    }

    std::unique_ptr<TypeNode> result_type = nullptr;

    switch (expr->node_type) {
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION: {
            result_type =
                visitIntegerLiteralExpression(static_cast<IntegerLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION: {
            result_type =
                visitStringLiteralExpression(static_cast<StringLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION: {
            result_type =
                visitBooleanLiteralExpression(static_cast<BooleanLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION: {
            result_type =
                visitCharacterLiteralExpression(static_cast<CharacterLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION: {
            result_type =
                visitFloatLiteralExpression(static_cast<FloatLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION: {
            result_type =
                visitDoubleLiteralExpression(static_cast<DoubleLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::COMPLEX_LITERAL_EXPRESSION: {
            result_type =
                visitComplexLiteralExpression(static_cast<ComplexLiteralExpressionNode*>(expr));
            break;
        }
        case ASTNode::NodeType::VARIABLE_REFERENCE: {
            auto* var_node = static_cast<VariableReferenceNode*>(expr);
            visit(var_node);
            Symbol* sym = symbolTable.lookup(var_node->name);
            if ((sym == nullptr) || !sym->dataType) {
                Logger::report_error("Semantic Error", "Variable not found or unresolved.",
                                     var_node->line);
            }
            result_type = sym->dataType->clone();
            break;
        }
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            auto* bin_node = static_cast<BinaryOperationExpressionNode*>(expr);
            visit(bin_node);
            if (!bin_node->resolved_type) {
                Logger::report_error("Semantic Error", "Binary op failed type resolution",
                                     bin_node->line);
            }

            result_type = bin_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::GATE_APPLICATION_OPERATION_EXPRESSION: {
            auto* gate_node = static_cast<GateAppOperationExpressionNode*>(expr);
            visit(gate_node);
            if(!gate_node->resolved_type) {
                Logger::report_error("Semantic Error", "Gate application op failed type resolution",
                        gate_node->line);
            }

            result_type = gate_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::FUNCTION_CALL: {
            auto* func_node = static_cast<FunctionCallNode*>(expr);
            visit(func_node);

            if (func_node->function_name == "__builtin_sqrt" ||
                func_node->function_name == "__builtin_abs" ||
                func_node->function_name == "__builtin_round") {
                expr->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_DOUBLE);
                return expr->resolved_type->clone();
            } else if (func_node->function_name == "exit") {
                expr->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
                return expr->resolved_type->clone();
            }

            Symbol* func_symbol =
                symbolTable.lookup(static_cast<FunctionCallNode*>(expr)->function_name);
            if (func_symbol == nullptr) {
                Logger::report_error("Semantic Error",
                                     "Function '" +
                                         static_cast<FunctionCallNode*>(expr)->function_name +
                                         "' not found.",
                                     func_node->line);
            }
            func_node->resolved_type = func_symbol->dataType->clone();
            result_type = func_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION: {
            auto* member_node = static_cast<MemberAccessNode*>(expr);
            visit(member_node);
            return member_node->resolved_type->clone();
        }
        case ASTNode::NodeType::UNARY_OP_EXPRESSION: {
            auto* unary_node = static_cast<UnaryOpExpressionNode*>(expr);
            visit(unary_node);
            if (!unary_node->resolved_type) {
                Logger::report_error("Semantic Error", "Unary op failed type resolution",
                                     unary_node->line);
            }

            result_type = unary_node->resolved_type->clone();
            break;
        }
        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION: {
            visit(static_cast<ArrayAccessNode*>(expr));
            std::unique_ptr<TypeNode> array_type =
                visitExpression(static_cast<ArrayAccessNode*>(expr)->array_expr.get());
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
        case ASTNode::NodeType::SCOPE_RESOLUTION: {
            auto* scope_node = static_cast<ScopeResolutionNode*>(expr);
            visit(scope_node);
            if (!scope_node->resolved_type) {
                Logger::report_error("Semantic Error",
                                     "Could not resolve type for namespace member.",
                                     scope_node->line);
            }
            result_type = scope_node->resolved_type->clone();
            break;
        }
        default:
            Logger::report_error("Semantic Error", "Unexpected AST node type in visitExpression.");
    }

    if (result_type) {
        expr->resolved_type = result_type->clone();
        return result_type;
    }

    Logger::report_error("Semantic Error", "Expression resolution returned null.");
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitIntegerLiteralExpression(
    IntegerLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_INT);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitStringLiteralExpression(
    StringLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_STRING);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitBooleanLiteralExpression(
    BooleanLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitCharacterLiteralExpression(
    CharacterLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_CHAR);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitFloatLiteralExpression(
    FloatLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_FLOAT);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitDoubleLiteralExpression(
    DoubleLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_DOUBLE);
    return node->resolved_type->clone();
}

std::unique_ptr<TypeNode> SemanticAnalyzer::visitComplexLiteralExpression(
    ComplexLiteralExpressionNode* node) {
    node->resolved_type = std::make_unique<PrimitiveTypeNode>(Token::KEYWORD_COMPLEX);
    return node->resolved_type->clone();
}

std::string SemanticAnalyzer::typeToString(const TypeNode* type) {
    if (type == nullptr) {
        return "null";
    }
    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            auto p = static_cast<const PrimitiveTypeNode*>(type);
            switch (p->primitive_type) {
                case Token::KEYWORD_INT:
                    return "int";
                case Token::KEYWORD_FLOAT:
                    return "float";
                case Token::KEYWORD_DOUBLE:
                    return "double";
                case Token::KEYWORD_BOOL:
                    return "bool";
                case Token::KEYWORD_CHAR:
                    return "char";
                case Token::KEYWORD_STRING:
                    return "string";
                case Token::KEYWORD_VOID:
                    return "void";
                default:
                    return "primitive";
            }
        }
        case TypeNode::TypeCategory::POINTER:
            return "pointer";
        case TypeNode::TypeCategory::ARRAY:
            return "array";
        case TypeNode::TypeCategory::STRUCT:
            return "struct " + static_cast<const StructTypeNode*>(type)->struct_name;
        default:
            return "unknown";
    }
}
