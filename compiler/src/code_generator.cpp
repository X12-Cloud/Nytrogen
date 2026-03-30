#include "code_generator.hpp"
#include <stdexcept>
#include <sstream>
#include <iomanip>

CodeGenerator::CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable)
: program_ast(ast), symbolTable(symTable), string_label_counter(0) {}

void CodeGenerator::generate(const std::string& output_filename, bool is_entry_point) {
    out.open(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }

    // print formats
    constants.push_back({"_print_int_format", "db", "\"%d\", 10, 0"});
    constants.push_back({"_print_str_format", "db", "\"%s\", 10, 0"});
    constants.push_back({"_print_char_format", "db", "\"%c\", 10, 0"});
    constants.push_back({"_print_float_format", "db", "\"%f\", 10, 0"});

    out << "section .text" << std::endl;
    out << "extern printf" << std::endl;
    out << "extern strcmp" << std::endl;
    if (is_entry_point) out << "global _start" << std::endl;

    for (const auto& func : program_ast->functions) {
        if (!func->body_statements.empty()) out << "global " << func->name << std::endl;
	    else out << "extern " << func->name << std::endl;
    }

    // entry point
    if (is_entry_point) {
	    out << "_start:" << std::endl;
	    out << "  call main" << std::endl;
	    out << "  mov rdi, rax" << std::endl;
	    out << "  mov rax, 60" << std::endl;
	    out << "  syscall" << std::endl;
    }

    visit(program_ast.get());

    // print the .data section
    out << "\nsection .data" << std::endl;
    for (const auto& c : constants) {
        out << "    " << c.label << " " << c.type << " " << c.value << std::endl;
    }

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
        case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION:
            visit(static_cast<FloatLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION:
            visit(static_cast<DoubleLiteralExpressionNode*>(node));
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
            throw std::runtime_error("Code Generation Error: Unknown AST node type.");
    }
}

bool is_lvalue;

void CodeGenerator::visit(ProgramNode* node) {
    for (const auto& stmt : node->statements) {
        if (stmt->node_type == ASTNode::NodeType::VARIABLE_DECLARATION) {
            auto decl_node = static_cast<VariableDeclarationNode*>(stmt.get());
            for (const auto& decl : decl_node->declarations) {
                out << decl.name << ": resb " << getTypeSize(decl_node->type.get()) << std::endl;
            }
        }
    }
    for (const auto& func : node->functions) {
        visit(func.get());
    }
}

void CodeGenerator::visit(FunctionDefinitionNode* node) {
    current_function_name = node->name;
    if (node->is_extern) {
        out << "extern " << node->name << std::endl;
        return; // No further code generation for extern functions
    }

    out << node->name << ":" << std::endl;
    emit("push", "rbp");
    emit("mov", "rbp", "rsp");

    emit("and", "rsp", "-16");

    std::stringstream body_buffer;
    std::streambuf* backup = out.std::ios::rdbuf(body_buffer.rdbuf());

    // Generate code for all statements
    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    out.std::ios::rdbuf(backup);

    // Calculate total local variable space from current scope
    int local_var_space = 0;
    if (!symbolTable.all_scopes.empty()) local_var_space = -symbolTable.all_scopes.back()->currentOffset;
    if (local_var_space == 0) local_var_space = 64;

    int aligned_space = (local_var_space + 15) & ~15;
    if (aligned_space > 0) emit("sub", "rsp", std::to_string(aligned_space));

    // Push register arguments onto the stack
    const std::vector<std::string> arg_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int register_args_size = 0;
    for (int i = 0; i < node->parameters.size() && i < arg_registers.size(); ++i) {
        int offset = (i + 1) * -8; 
        out << "    mov [rbp + " << offset << "], " << arg_registers[i] << std::endl;
    }

    out << body_buffer.str();

    out << current_function_name << "_epilogue:" << std::endl;

    emit("leave");
    emit("ret");

    current_function_name = "";
}

void CodeGenerator::visit(ConstantDeclarationNode* node) {
    // No code generation needed for constant declarations
}

void CodeGenerator::visit(EnumStatementNode* node) {
    // No code generation needed for enum declarations
}

void CodeGenerator::visit(VariableDeclarationNode* node) {
    auto prim = static_cast<PrimitiveTypeNode*>(node->type.get());
    bool is_float = prim && (prim->primitive_type == Token::KEYWORD_FLOAT);
    bool is_double = prim && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    for (auto& decl : node->declarations) {
        Symbol* symbol = decl.resolved_symbol;
        if (!symbol) {
            throw std::runtime_error("Code generation error: variable '" + decl.name + "' not found in symbol table.");
        }

        if (node->type->category == TypeNode::TypeCategory::STRUCT) continue;
        if (decl.initial_value) {
	        visit(decl.initial_value.get());
	        emit_adv(node->type, "rbp", symbol->offset, (is_float || is_double) ? "xmm0" : "rax");
        }
    }
}

void CodeGenerator::visit(VariableAssignmentNode* node) {
    auto type = node->left->resolved_type;
    bool is_fp = isFloatingPoint(type);
    auto literal = dynamic_cast<LiteralExpressionNode*>(node->right.get());

    if (literal && !is_fp) {
	    is_lvalue = true;
	    visit(node->left.get());
	    is_lvalue = false;
	
	    std::string val = literal->getValueAsString();

	    emit_adv(type, "rax", 0, val);
	    return;
    }

    visit(node->right.get());
    if (is_fp) {
	    is_lvalue = true;
	    visit(node->left.get());
	    is_lvalue = false;

	    emit_adv(type, "rax", 0, "xmm0");
    } else {
        emit("push", "rax");

	    is_lvalue = true;
	    visit(node->left.get());
	    is_lvalue = false;
	
        emit("pop", "rbx");
	
	    emit_adv(type, "rax", 0, "rbx");
    }
}

void CodeGenerator::visit(VariableReferenceNode* node) {
    Symbol* symbol = node->resolved_symbol;
    int offset = symbol->offset;

    if (!node->resolved_symbol) {
        throw std::runtime_error("CodeGen Error: Symbol not resolved for " + node->name);
    }
    if (!node->resolved_symbol->dataType) {
        throw std::runtime_error("CodeGen Error: Variable '" + node->name + "' has NO TYPE in symbol table!");
    }

    if (!symbol) {
        throw std::runtime_error("CodeGen Error: Reference to '" + node->name + "' not resolved.");
    }

    if (symbol->type == Symbol::SymbolType::CONSTANT) {
        visit(symbol->value.get());
        return;
    }

    auto prim = dynamic_cast<PrimitiveTypeNode*>(node->resolved_type.get());
    bool is_double = prim && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = prim && (prim->primitive_type == Token::KEYWORD_FLOAT);
	
    if (is_lvalue) {
        emit("lea", "rax", "[rbp + " + std::to_string(offset) + "]");
    } else {
	    load_adv(node->resolved_type, (is_float || is_double) ? "xmm0" : "rax", "rbp", offset);
    }
}

void CodeGenerator::visit(BinaryOperationExpressionNode* node) {
    // Left
    visit(node->left.get());
    auto prim = dynamic_cast<PrimitiveTypeNode*>(node->resolved_type.get());
    bool is_double = prim && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = prim && (prim->primitive_type == Token::KEYWORD_FLOAT);

    if (is_float || is_double) {
        out << "    sub rsp, 8" << std::endl;
        if (is_double) out << "    vmovsd qword [rsp], xmm0" << std::endl;
	    else out << "    vmovss dword [rsp], xmm0" << std::endl;
    } else out << "    push rax" << std::endl;

    // Right
    visit(node->right.get());

    if (is_float || is_double) {
	if (is_double) out << "    vmovsd xmm1, qword [rsp]" << std::endl;
	else out << "    vmovss xmm1, dword [rsp]" << std::endl;
        out << "    add rsp, 8" << std::endl; // Left is in xmm1, Right is in xmm0
    } else {
        out << "    pop rbx" << std::endl; 
	    out << "    mov rcx, rbx" << std::endl; // Left is in rbx, Right is in rax
    }

    char type = 'd';
    if (is_float) type = 'f';
    else if (is_double) type = 'l';
    switch (node->op_type) {
        case Token::PLUS:
	        emit_binary_op("add", type);
            break;
        case Token::MINUS:
	        emit_binary_op("sub", type);
            break;
        case Token::STAR:
	        emit_binary_op("imul", type);
            break;
        case Token::SLASH:
	        emit_binary_op("idiv", type);
            break;
        case Token::EQUAL_EQUAL:
            if (node->left->resolved_type && node->left->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE && static_cast<PrimitiveTypeNode*>(node->left->resolved_type.get())->primitive_type == Token::KEYWORD_STRING) {
                // String comparison
                out << "    mov rdi, rcx" << std::endl;
                out << "    mov rsi, rax" << std::endl;
                out << "    call strcmp" << std::endl;
                out << "    test rax, rax" << std::endl;
                out << "    sete al" << std::endl;
                out << "    movzx rax, al" << std::endl;
            } else {
                // Integer/pointer comparison
                out << "    cmp rcx, rax" << std::endl;
                out << "    sete al" << std::endl;
                out << "    movzx rax, al" << std::endl;
            }
            break;
        case Token::BANG_EQUAL:
            if (node->left->resolved_type && node->left->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE && static_cast<PrimitiveTypeNode*>(node->left->resolved_type.get())->primitive_type == Token::KEYWORD_STRING) {
                // String comparison
                out << "    mov rdi, rcx" << std::endl;
                out << "    mov rsi, rax" << std::endl;
                out << "    call strcmp" << std::endl;
                out << "    test rax, rax" << std::endl;
                out << "    setne al" << std::endl;
                out << "    movzx rax, al" << std::endl;
            } else {
                // Integer/pointer comparison
                out << "    cmp rcx, rax" << std::endl;
                out << "    setne al" << std::endl;
                out << "    movzx rax, al" << std::endl;
            }
            break;
        case Token::LESS:
            out << "    cmp rcx, rax" << std::endl;
            out << "    setl al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::GREATER:
            out << "    cmp rcx, rax" << std::endl;
            out << "    setg al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::LESS_EQUAL:
            out << "    cmp rcx, rax" << std::endl;
            out << "    setle al" << std::endl;
            out << "    movzx rax, al" << std::endl;
            break;
        case Token::GREATER_EQUAL:
            out << "    cmp rcx, rax" << std::endl;
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

        auto expr_type = expr->resolved_type;
        if (!expr_type) {
             // Fallback if semantic analysis missed a type
             out << "    mov rsi, rax" << std::endl;
             out << "    lea rdi, [rel _print_int_format]" << std::endl;
             out << "    xor rax, rax" << std::endl;
             out << "    call printf" << std::endl;
             continue;
        }
        emit_print(expr_type);
    }
}

void CodeGenerator::visit(ReturnStatementNode* node) {
    if (node->expression) {
        visit(node->expression.get());
        if (!node->resolved_type) {
            throw std::runtime_error("CodeGen Error: Return statement has an expression but no resolved type.");
        }
        emit("pop", "rax");
    }
    out << "    jmp " << current_function_name << "_epilogue" << std::endl;
}

void CodeGenerator::visit(IfStatementNode* node) {
    static int if_counter = 0;
    int label_id = if_counter++;

    std::string true_label = "_if_true_" + std::to_string(label_id);
    std::string false_label = "_if_false_" + std::to_string(label_id);
    std::string end_label = "_if_end_" + std::to_string(label_id);

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

    std::string start_label = "_while_start_" + std::to_string(label_id);
    std::string end_label = "_while_end_" + std::to_string(label_id);

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

    std::string loop_start_label = "_for_loop_start_" + std::to_string(label_id);
    std::string loop_condition_label = "_for_loop_condition_" + std::to_string(label_id);
    std::string loop_end_label = "_for_loop_end_" + std::to_string(label_id);

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
    const std::vector<std::string> arg_regs_64 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    const std::vector<std::string> arg_regs_32 = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
    int arg_count = node->arguments.size();

    for (int i = arg_count - 1; i >= (int)arg_regs_64.size(); --i) {
        visit(node->arguments[i].get());
        out << "    push rax" << std::endl;
    }

    for (int i = std::min(arg_count, (int)arg_regs_64.size()) - 1; i >= 0; --i) {
        visit(node->arguments[i].get());

        int size = 8;
        if (node->arguments[i]->resolved_type) size = getTypeSize(node->arguments[i]->resolved_type.get());
        else std::cerr << "Warning: Argument " << i << " in call to '" << node->function_name << "' has no resolved type. Defaulting to 8 bytes." << std::endl;

        if (size == 8) out << "    mov " << arg_regs_64[i] << ", rax" << std::endl;
        else out << "    mov " << arg_regs_32[i] << ", eax" << std::endl;
    }

    out << "    call " << node->function_name << std::endl;

    if (arg_count > arg_regs_64.size()) {
        out << "    add rsp, " << (arg_count - arg_regs_64.size()) * 8 << std::endl;
    }
}

void CodeGenerator::visit(MemberAccessNode* node) {
    bool old_lvalue = is_lvalue;
    is_lvalue = true; 
    visit(node->struct_expr.get()); 
    is_lvalue = old_lvalue; // Restore state

    Symbol* member_symbol = node->resolved_symbol;
    if (member_symbol && member_symbol->offset != 0) {
        out << "    add rax, " << member_symbol->offset << std::endl;
    }

    if (!node->resolved_type) {
        throw std::runtime_error("CodeGen Error: Member access '" + node->member_name + "' has no resolved type.");
    }

    int size = getTypeSize(node->resolved_type.get()); 

    if (!is_lvalue) {
        if (size == 4) {
            out << "    movsx rax, dword [rax]" << std::endl;
        } else if (size == 1) {
            out << "    movsx rax, byte [rax]" << std::endl;
        } else {
            out << "    mov rax, [rax]" << std::endl;
        }
    }
    // out << "    add rax, " << member_symbol->offset << std::endl;
    // out << "    mov eax, " << "dword [rax]" << std::endl;
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
    } else if (node->op_type == Token::BANG) {
	visit(node->operand.get());
	out << "    test rax, rax" << std::endl;
	out << "    setz al" << std::endl;
	out << "    movzx rax, al" << std::endl;
    }
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    bool was_lvalue = is_lvalue;
    is_lvalue = false;
    visit(node->index_expr.get());
    out << "    mov rbx, rax" << std::endl;
    is_lvalue = was_lvalue;

    int element_size = 8; 
    if (node->array_expr && node->array_expr->resolved_type) {
        if (node->array_expr->resolved_type->category == TypeNode::TypeCategory::ARRAY) {
            auto arr_type = static_cast<ArrayTypeNode*>(node->array_expr->resolved_type.get());
            element_size = getTypeSize(arr_type->base_type.get());
        }
    }

    if (node->array_expr->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
        auto var_ref = static_cast<VariableReferenceNode*>(node->array_expr.get());
        Symbol* symbol = var_ref->resolved_symbol; 
        
        if (symbol) {
            out << "    lea rax, [rbp + " << symbol->offset << "]" << std::endl;
        } else {
             throw std::runtime_error("CodeGen Error: Symbol not found.");
        }
    } else {
        visit(node->array_expr.get());
    }

    out << "    imul rbx, " << element_size << std::endl;
    out << "    add rax, rbx" << std::endl;

    if (!was_lvalue) {
        if (element_size == 4) {
             out << "    movsx rax, dword [rax]" << std::endl;
        } else {
             out << "    mov rax, [rax]" << std::endl;
        }
    }
}

void CodeGenerator::visit(StructDefinitionNode* node) {
    // No code generation needed for struct definitions
}

void CodeGenerator::visit(IntegerLiteralExpressionNode* node) {
    out << "    mov rax, " << node->value << std::endl;

    if (!node->resolved_type) node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_INT);
}

void CodeGenerator::visit(FloatLiteralExpressionNode* node) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(6) << node->value;
    std::string val_str = ss.str();

    if (constants_map.find(val_str) == constants_map.end()) {
        std::string label = "_float_" + std::to_string(string_label_counter++);
        constants_map[val_str] = label;
        constants.push_back({label, "dd", val_str});
    }

    out << "    vmovss xmm0, [rel " << constants_map[val_str] << "]" << std::endl;
}

void CodeGenerator::visit(DoubleLiteralExpressionNode* node) {
    std::stringstream ss;
    ss << std::fixed << std::setprecision(15) << node->value;
    std::string val_str = ss.str();

    if (constants_map.find(val_str) == constants_map.end()) {
        std::string label = "_double_" + std::to_string(string_label_counter++);
        constants_map[val_str] = label;
        constants.push_back({label, "dq", val_str});
    }

    out << "    vmovsd xmm0, " << "qword [rel " << node->label << "]" << std::endl;
}

void CodeGenerator::visit(StringLiteralExpressionNode* node) {
    std::string formatted_val = "\"" + node->value + "\", 0";
    if (constants_map.find(formatted_val) == constants_map.end()) {
        std::string label = "_str_" + std::to_string(string_label_counter++);
	constants_map[formatted_val] = label;
        constants.push_back({label, "db", formatted_val});
    }

    std::string label = constants_map[formatted_val];
    out << "    lea rax, [rel " << label << "]" << std::endl;
    node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_STRING);
}

void CodeGenerator::visit(BooleanLiteralExpressionNode* node) {
    out << "    mov rax, " << (node->value ? 1 : 0) << std::endl;
}

void CodeGenerator::visit(CharacterLiteralExpressionNode* node) {
    out << "    mov rax, " << static_cast<int>(node->value) << std::endl;
    if (!node->resolved_type) node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
}

void CodeGenerator::visit(AsmStatementNode* node) {
    for (const auto& line : node->lines) {
        out << "    " << line << "\n";
    }
}

int CodeGenerator::getTypeSize(const TypeNode* type) {
    if (!type) {
        std::cerr << "Type is null" << std::endl;
        throw std::runtime_error("Code Generation Error: Attempted to get size of a null type.");
    }

    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const PrimitiveTypeNode* prim_type = static_cast<const PrimitiveTypeNode*>(type);
            switch (prim_type->primitive_type) {
                case Token::KEYWORD_INT: return 4;
                case Token::KEYWORD_BOOL: return 1;
                case Token::KEYWORD_CHAR: return 1;
                case Token::KEYWORD_STRING: return 8;
                case Token::KEYWORD_VOID: return 0;
                case Token::KEYWORD_FLOAT: return 4;
                case Token::KEYWORD_DOUBLE: return 8;
                default: throw std::runtime_error("Code Generation Error: Unknown primitive type (" + std::to_string((int)prim_type->primitive_type) + ") for size calculation. (Type category: " + std::to_string((int)type->category) + ")");
            }
        }
        case TypeNode::TypeCategory::POINTER: return 8;
        case TypeNode::TypeCategory::ARRAY: {
            const ArrayTypeNode* array_type = static_cast<const ArrayTypeNode*>(type);
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
                 throw std::runtime_error("Code Generation Error: Undefined struct '" + struct_type->struct_name + "'.");
              }
              return struct_def_symbol->structDef->size;
          }
          default: throw std::runtime_error("Code Generation Error: Unknown type category for size calculation.");
    }
}
