
#include "code_generator.hpp"
#include <stdexcept>

CodeGenerator::CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable)
    : program_ast(ast), symbolTable(symTable), string_label_counter(0) {}

void CodeGenerator::generate(const std::string& output_filename) {
    out.open(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }

    out << "section .data" << std::endl;
    out << "  _print_int_format db \"%d\", 10, 0" << std::endl;
    out << "  _print_str_format db \"%s\", 10, 0" << std::endl;
    out << "  _print_char_format db \"%c\", 10, 0" << std::endl;

    out << "section .text" << std::endl;
    out << "global _start" << std::endl;
    out << "extern printf" << std::endl;

    visit(program_ast.get());

    out << "_start:" << std::endl;
    out << "  call main" << std::endl;
    out << "  mov rdi, rax" << std::endl;
    out << "  mov rax, 60" << std::endl;
    out << "  syscall" << std::endl;

    out.close();
}

void CodeGenerator::visit(ASTNode* node) {
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
            visit(static_cast<IntegerLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
            visit(static_cast<StringLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
            visit(static_cast<BooleanLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            visit(static_cast<CharacterLiteralExpressionNode*>(node));
            break;
        default:
            throw std::runtime_error("Code Generation Error: Unknown AST node type.");
    }
}

void CodeGenerator::visit(ProgramNode* node) {
    for (const auto& func : node->functions) {
        visit(func.get());
    }
}

void CodeGenerator::visit(FunctionDefinitionNode* node) {
    out << node->name << ":" << std::endl;
    out << "    push rbp" << std::endl;
    out << "    mov rbp, rsp" << std::endl;

    symbolTable.enterScope();

    int local_var_space = 0;
    for (const auto& stmt : node->body_statements) {
        if (stmt->node_type == ASTNode::NodeType::VARIABLE_DECLARATION) {
            auto decl = static_cast<VariableDeclarationNode*>(stmt.get());
            int var_size = 0;
            if (decl->type->category == TypeNode::TypeCategory::STRUCT) {
                Symbol* struct_def_symbol = symbolTable.lookup(static_cast<StructTypeNode*>(decl->type.get())->struct_name);
                if (struct_def_symbol) {
                    var_size = struct_def_symbol->structDef->size;
                }
            } else {
                Symbol* symbol = symbolTable.lookup(decl->name);
                if (symbol) {
                    var_size = symbol->size;
                }
            }
            symbolTable.scopes.back()->currentOffset -= var_size;
            int offset = symbolTable.scopes.back()->currentOffset;
            Symbol symbol(Symbol::SymbolType::VARIABLE, decl->name, decl->type->clone(), offset, var_size);
            symbolTable.addSymbol(std::move(symbol));
            local_var_space += var_size;
        }
    }
    out << "    sub rsp, " << local_var_space << std::endl;

    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    if (node->name == "main") {
        out << ".main_epilogue:" << std::endl;
    }

    out << "    mov rsp, rbp" << std::endl;
    out << "    pop rbp" << std::endl;
    out << "    ret" << std::endl << std::endl;

    symbolTable.exitScope();
}

void CodeGenerator::visit(VariableDeclarationNode* node) {
    Symbol* symbol = symbolTable.lookup(node->name);
    if (!symbol) {
        throw std::runtime_error("Code generation error: variable '" + node->name + "' not found in symbol table.");
    }

    if (node->type->category == TypeNode::TypeCategory::STRUCT) {
        // For structs, we just need to make sure space is allocated.
        // The function prologue already does this.
    } else if (node->initial_value) {
        visit(node->initial_value.get());
        out << "    mov [rbp + " << symbol->offset << "], rax" << std::endl;
    }
}

void CodeGenerator::visit(VariableAssignmentNode* node) {
    visit(node->right.get());
    out << "    push rax" << std::endl;
    visit(node->left.get());
    out << "    pop rbx" << std::endl;

    if (node->left->node_type == ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION) {
        out << "    mov [rax], rbx" << std::endl;
    } else {
        out << "    mov [rax], rbx" << std::endl;
    }
}

void CodeGenerator::visit(VariableReferenceNode* node) {
    Symbol* symbol = symbolTable.lookup(node->name);
    if (symbol) {
        out << "    lea rax, [rbp + " << symbol->offset << "]" << std::endl;
    }
}

void CodeGenerator::visit(BinaryOperationExpressionNode* node) {
    visit(node->left.get());
    out << "    push rax" << std::endl;
    visit(node->right.get());
    out << "    pop rbx" << std::endl;

    switch (node->op_type) {
        case Token::PLUS:
            out << "    add rbx, rax" << std::endl;
            out << "    mov rax, rbx" << std::endl;
            break;
        case Token::MINUS:
            out << "    sub rbx, rax" << std::endl;
            out << "    mov rax, rbx" << std::endl;
            break;
        case Token::STAR:
            out << "    imul rax, rbx" << std::endl;
            break;
        case Token::SLASH:
            out << "    mov rcx, rax" << std::endl;
            out << "    mov rax, rbx" << std::endl;
            out << "    mov rbx, rcx" << std::endl;
            out << "    xor rdx, rdx" << std::endl;
            out << "    idiv rbx" << std::endl;
            break;
        case Token::EQUAL_EQUAL:
            out << "    cmp rbx, rax" << std::endl;
            out << "    sete al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::BANG_EQUAL:
            out << "    cmp rbx, rax" << std::endl;
            out << "    setne al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::LESS:
            out << "    cmp rbx, rax" << std::endl;
            out << "    setl al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::GREATER:
            out << "    cmp rbx, rax" << std::endl;
            out << "    setg al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::LESS_EQUAL:
            out << "    cmp rbx, rax" << std::endl;
            out << "    setle al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::GREATER_EQUAL:
            out << "    cmp rbx, rax" << std::endl;
            out << "    setge al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        default:
            throw std::runtime_error("Unknown binary operator.");
    }
}

void CodeGenerator::visit(PrintStatementNode* node) {
    for (const auto& expr : node->expressions) {
        visit(expr.get());

        if (expr->node_type == ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION) {
            out << "    mov rax, [rax]" << std::endl;
        }

        if (expr->node_type == ASTNode::NodeType::STRING_LITERAL_EXPRESSION) {
            out << "    mov rsi, rax" << std::endl;
            out << "    lea rdi, [rel _print_str_format]" << std::endl;
        } else if (expr->node_type == ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION) {
            out << "    mov rsi, rax" << std::endl;
            out << "    lea rdi, [rel _print_char_format]" << std::endl;
        } else {
            out << "    mov rsi, rax" << std::endl;
            out << "    lea rdi, [rel _print_int_format]" << std::endl;
        }

        out << "    xor rax, rax" << std::endl;
        out << "    call printf" << std::endl;
    }
}

void CodeGenerator::visit(ReturnStatementNode* node) {
    visit(node->expression.get());
}

void CodeGenerator::visit(IfStatementNode* node) {
    static int if_counter = 0;
    int label_id = if_counter++;

    std::string true_label = ".if_true_" + std::to_string(label_id);
    std::string false_label = ".if_false_" + std::to_string(label_id);
    std::string end_label = ".if_end_" + std::to_string(label_id);

    visit(node->condition.get());
    out << "    cmp rax, 0" << std::endl;
    out << "    je " << false_label << std::endl;

    out << true_label << ":" << std::endl;
    for (const auto& stmt : node->true_block) {
        visit(stmt.get());
    }
    out << "    jmp " << end_label << std::endl;

    out << false_label << ":" << std::endl;
    for (const auto& stmt : node->false_block) {
        visit(stmt.get());
    }

    out << end_label << ":" << std::endl;
}

void CodeGenerator::visit(WhileStatementNode* node) {
    static int while_counter = 0;
    int label_id = while_counter++;

    std::string start_label = ".while_start_" + std::to_string(label_id);
    std::string end_label = ".while_end_" + std::to_string(label_id);

    out << start_label << ":" << std::endl;
    visit(node->condition.get());
    out << "    cmp rax, 0" << std::endl;
    out << "    je " << end_label << std::endl;

    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }

    out << "    jmp " << start_label << std::endl;
    out << end_label << ":" << std::endl;
}

void CodeGenerator::visit(ForStatementNode* node) {
    static int for_counter = 0;
    int label_id = for_counter++;

    std::string loop_start_label = ".for_loop_start_" + std::to_string(label_id);
    std::string loop_condition_label = ".for_loop_condition_" + std::to_string(label_id);
    std::string loop_end_label = ".for_loop_end_" + std::to_string(label_id);

    if (node->initializer) {
        visit(node->initializer.get());
    }

    out << loop_condition_label << ":" << std::endl;
    if (node->condition) {
        visit(node->condition.get());
        out << "    cmp rax, 0" << std::endl;
        out << "    je " << loop_end_label << std::endl;
    }

    out << loop_start_label << ":" << std::endl;
    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }
    if (node->increment) {
        visit(node->increment.get());
    }

    out << "    jmp " << loop_condition_label << std::endl;
    out << loop_end_label << ":" << std::endl;
}

void CodeGenerator::visit(FunctionCallNode* node) {
    for (int i = node->arguments.size() - 1; i >= 0; --i) {
        visit(node->arguments[i].get());
        out << "    push rax" << std::endl;
    }
    out << "    call " << node->function_name << std::endl;
    if (!node->arguments.empty()) {
        out << "    add rsp, " << node->arguments.size() * 8 << std::endl;
    }
}

void CodeGenerator::visit(MemberAccessNode* node) {
    visit(node->struct_expr.get()); // Get the base address of the struct in rax

    Symbol* member_symbol = node->resolved_symbol;
    if (!member_symbol) {
        throw std::runtime_error("Code generation error: member symbol not resolved for '" + node->member_name + "'.");
    }

    out << "    lea rbx, [rax + " << member_symbol->offset << "]" << std::endl;
    out << "    mov rax, rbx" << std::endl;
}

void CodeGenerator::visit(UnaryOpExpressionNode* node) {
    visit(node->operand.get());
    if (node->op_type == Token::ADDRESSOF) {
        const auto* ref_node = static_cast<const VariableReferenceNode*>(node->operand.get());
        Symbol* var_symbol = ref_node->resolved_symbol;
        if (!var_symbol) {
            throw std::runtime_error("Code generation error: variable '" + ref_node->name + "' used before declaration for address-of (resolved_symbol is null).");
        }
        int offset = var_symbol->offset;
        out << "    lea rax, [rbp + " << std::to_string(offset) << "]" << std::endl;
    } else if (node->op_type == Token::STAR) {
        out << "    mov rax, [rax]" << std::endl;
    }
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    visit(node->index_expr.get());
    out << "    mov rbx, rax" << std::endl;
    const auto* ref_node = static_cast<const VariableReferenceNode*>(node->array_expr.get());
    Symbol* var_symbol = ref_node->resolved_symbol;
    if (!var_symbol) {
        throw std::runtime_error("Code generation error: array variable '" + ref_node->name + "' not found for access (resolved_symbol is null).");
    }
    int offset = var_symbol->offset;
    out << "    lea rcx, [rbp + " << std::to_string(offset) + "]" << std::endl;
    out << "    imul rbx, 8" << std::endl;
    out << "    add rcx, rbx" << std::endl;
    out << "    mov rax, [rcx]" << std::endl;
}

void CodeGenerator::visit(StructDefinitionNode* node) {
    // No code generation needed for struct definitions
}

void CodeGenerator::visit(IntegerLiteralExpressionNode* node) {
    out << "    mov rax, " << node->value << std::endl;
}

void CodeGenerator::visit(StringLiteralExpressionNode* node) {
    std::string label = "_str_" + std::to_string(string_label_counter++);
    out << "section .data" << std::endl;
    out << label << " db \"" << node->value << "\", 0" << std::endl;
    out << "section .text" << std::endl;
    out << "    lea rax, [rel " << label << "]" << std::endl;
}

void CodeGenerator::visit(BooleanLiteralExpressionNode* node) {
    out << "    mov rax, " << (node->value ? 1 : 0) << std::endl;
}

void CodeGenerator::visit(CharacterLiteralExpressionNode* node) {
    out << "    mov rax, " << static_cast<int>(node->value) << std::endl;
}
