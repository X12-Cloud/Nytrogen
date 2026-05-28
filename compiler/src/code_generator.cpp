#include "code_generator.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

CodeGenerator::CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable)
    : program_ast(ast), symbolTable(symTable), string_label_counter(0) {}

std::string unescapeString(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            switch (input[i + 1]) {
                case 'n': result += "\", 10, \""; break;
                case 't': result += "\", 9, \""; break;
                case 'r': result += "\", 13, \""; break;
                case '"': result += "\", 34, \""; break;
                case '\\': result += "\", 92, \""; break;
                default:
                    result += input[i];
                    result += input[i + 1];
                    break;
            }
            i++;
        } else {
            result += input[i];
        }
    }
    return result;
}

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
    if (is_entry_point) {
        out << "global _start" << std::endl;
    }

    for (const auto& func : program_ast->functions) {
        if (!func->body_statements.empty()) {
            out << "global " << func->name << std::endl;
        } else {
            out << "extern " << func->name << std::endl;
        }
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
    std::unordered_set<std::string> emitted_data_labels;
    for (const auto& c : constants) {
        if (emitted_data_labels.find(c.label) == emitted_data_labels.end()) {
            out << "    " << c.label << " " << c.type << " " << c.value << std::endl;
            emitted_data_labels.insert(c.label);
        }
    }

    out.close();
}

void CodeGenerator::visit(ASTNode* node) {
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
        case ASTNode::NodeType::NAMESPACE_DEFINITION:
            visit(static_cast<NamespaceDefinition*>(node));
            break;
        case ASTNode::NodeType::SCOPE_RESOLUTION:
            visit(static_cast<ScopeResolutionNode*>(node));
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
        visit(stmt.get());
    }
    for (const auto& func : node->functions) {
        visit(func.get());
    }
}

void CodeGenerator::visit(FunctionDefinitionNode* node) {
    current_function_name = node->mangled_name;
    current_stack_depth = 0;
    if (node->is_extern) {
        out << "extern " << node->mangled_name << std::endl;
        return;  // No further code generation for extern functions
    }

    out << node->mangled_name << ":" << std::endl;
    emit("push", "rbp");
    current_stack_depth += 8;
    emit("mov", "rbp", "rsp");

    emit("and", "rsp", "-16");
    current_stack_depth = 0;

    std::stringstream body_buffer;
    std::streambuf* backup = out.std::ios::rdbuf(body_buffer.rdbuf());

    // Generate code for all statements
    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    out.std::ios::rdbuf(backup);

    // Calculate total local variable space from current scope
    int local_var_space = 0;
    if (!symbolTable.all_scopes.empty()) {
        local_var_space = -symbolTable.all_scopes.back()->currentOffset;
    }
    if (local_var_space == 0) {
        local_var_space = 64;
    }

    int aligned_space = (local_var_space + 15) & ~15;
    if (aligned_space > 0) {
        emit("sub", "rsp", std::to_string(aligned_space));
        current_stack_depth += aligned_space;
    }

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

void CodeGenerator::visit(NamespaceDefinition* node) {
    std::string old_ns = current_namespace_name;
    current_namespace_name = node->name;

    for (auto& m : node->members) {
        if (m.node) {
            visit(m.node.get());
        }
    }
    current_namespace_name = old_ns;
}

void CodeGenerator::visit(ScopeResolutionNode* node) {
    Symbol* ns_symbol = symbolTable.lookup(node->namespace_name);
    if ((ns_symbol != nullptr) && (ns_symbol->internal_scope != nullptr)) {
        auto it = ns_symbol->internal_scope->symbols.find(node->member->get_value());
        if (it != ns_symbol->internal_scope->symbols.end()) {
            Symbol& sym = it->second;
            if (sym.dataType) {
                node->member->resolved_type = sym.dataType->clone();
                if (node->member->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
                    auto* var = static_cast<VariableReferenceNode*>(node->member.get());
                    var->resolved_symbol = &sym;
                }
            }
        }
    }
    visit(node->member.get());
}

void CodeGenerator::visit(ConstantDeclarationNode* node) {
    // No code generation needed for constant declarations
}

void CodeGenerator::visit(EnumStatementNode* node) {
    // No code generation needed for enum declarations
}

void CodeGenerator::visit(VariableDeclarationNode* node) {
    auto prim = static_cast<PrimitiveTypeNode*>(node->type.get());
    bool is_float = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT);
    bool is_double = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_string = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_STRING);
    int size = getTypeSize(node->type.get());
    std::string asm_label;
    for (auto& decl : node->declarations) {
        Symbol* symbol = decl.resolved_symbol;
        if (symbol == nullptr) {
            throw std::runtime_error("Code generation error: variable '" + decl.name +
                                     "' not found in symbol table.");
        }

        std::string final_name = symbol->mangled_name;

        if (!final_name.empty() && final_name != decl.name) {
            std::string init_val = "0";
            bool has_non_const_init = false;

            if (decl.initial_value) {
                if (decl.initial_value->is_constant()) {
                    if (is_string) {
                        std::string str_data_label = "_str_var_data_" + std::to_string(string_label_counter++);
                        constants.push_back({str_data_label, "db", "\"" + unescapeString(decl.initial_value->get_value()) + "\", 0"});
                        init_val = str_data_label;
                    } else {
                        init_val = decl.initial_value->get_value();
                    }
                } else {
                    has_non_const_init = true;
                }
            }

            std::string nasm_type;
            std::string final_init_val = init_val;

            if (node->type && node->type->category == TypeNode::TypeCategory::ARRAY) {
                nasm_type = "times " + std::to_string(size);
                final_init_val = "db 0";
            } else {
                nasm_type = (size == 4)                 ? "dd"
                            : ( is_string || size == 8) ? "dq"
                            : (size == 1)               ? "db"
                                                        : "dw";
            }

            constants.push_back({final_name, nasm_type, final_init_val});

            if (has_non_const_init) {
                visit(decl.initial_value.get());
                if (is_float || is_double) {
                    std::string instr = is_float ? "vmovss" : "vmovsd";
                    out << "    " << instr << " [rel " << final_name << "], xmm0" << std::endl;
                } else {
                    out << "    mov [rel " << final_name << "], rax" << std::endl;
                }
            }
        } else {
            if (decl.initial_value) {
                visit(decl.initial_value.get());
                emit_adv(node->type, "rbp", symbol->offset,
                         (is_float || is_double) ? "xmm0" : "rax");
            }
        }
    }
}

void CodeGenerator::visit(VariableAssignmentNode* node) {
    auto type = node->left->resolved_type;
    bool is_fp = isFloatingPoint(type);

    visit(node->right.get());

    auto* var_ref = dynamic_cast<VariableReferenceNode*>(node->left.get());
    if ((var_ref != nullptr) && !var_ref->resolved_symbol->mangled_name.empty()) {
        std::string label = var_ref->resolved_symbol->mangled_name;
        if (is_fp) {
            std::string instr = (getTypeSize(type.get()) == 4) ? "vmovss" : "vmovsd";
            out << "    " << instr << " [rel " << label << "], xmm0" << std::endl;
        } else {
            int size = getTypeSize(type.get());
            if (size == 4) emit("mov", "dword [rel " + label + "]", "eax");
            else if (size == 1) emit("mov", "byte [rel " + label + "]", "al");
            else emit("mov", "qword [rel " + label + "]", "rax");
        }
    } else {
        RegisterAllocator::RegID tempReg = allocator.allocate();
        bool use_stack = (tempReg == RegisterAllocator::RegID::NONE);
        std::string tempRegName;

        if (use_stack) {
            // FALLBACK: Spill RHS to stack because we are out of registers
            emit("push", "rax");
            current_stack_depth += 8;
        } else {
            // NORMAL: Keep RHS in a temporary register
            tempRegName = allocator.get_name(tempReg, 8);
            emit("mov", tempRegName, "rax");
        }

        is_lvalue = true;
        visit(node->left.get()); 
        is_lvalue = false;

        int size = getTypeSize(type.get());
        std::string prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";
        std::string regToUse = (use_stack) ? allocator.get_name(RegisterAllocator::RBX, size)
                                           : allocator.get_name(tempReg, size);

        if (use_stack) {
            emit("pop", "rbx");
            current_stack_depth -= 8;
            std::string regToUse = allocator.get_name(RegisterAllocator::RBX, size);
            std::cout << "DEBUG: Assigning size " << size << " using register " << regToUse << std::endl;
            emit("mov", prefix + " [rax]", regToUse);
        } else {
            emit("mov", prefix + " [rax]", regToUse);
            allocator.free_reg(tempReg);
        }
    }
}

RegisterAllocator::RegID CodeGenerator::visit(VariableReferenceNode* node) {
    Symbol* sym = node->resolved_symbol;
    if (sym == nullptr) {
        throw std::runtime_error("Code Generation Error: Undefined variable " + node->name);
    }

    Symbol* symbol = node->resolved_symbol;
    if (symbol == nullptr) {
        throw std::runtime_error("CodeGen Error: Symbol not resolved for " + node->name);
    }
    if (!symbol->dataType) {
        throw std::runtime_error("CodeGen Error: Variable '" + node->name + "' has NO TYPE in symbol table!");
    }

    int offset = symbol->offset;

    if (symbol->type == Symbol::SymbolType::CONSTANT) {
        RegisterAllocator::RegID reg = allocator.allocate();
        std::string reg_name = reg_to_str(reg, node->resolved_type.get());

        if (auto lit_node = dynamic_cast<IntegerLiteralExpressionNode*>(symbol->value.get())) {
            emit("mov", reg_name, std::to_string(lit_node->value));
        } else {
            throw std::runtime_error("CodeGen Error: Constant type currently unhandled for direct evaluation");
        }

        return reg;
    }
    RegisterAllocator::RegID reg = allocator.allocate();
    std::string reg_name = reg_to_str(reg, node->resolved_type.get());

    auto prim = dynamic_cast<PrimitiveTypeNode*>(node->resolved_type.get());
    bool is_double = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT);
    bool is_global = !symbol->mangled_name.empty() && symbol->mangled_name != symbol->name;

    if (is_global) {
        std::string asm_label = symbol->mangled_name;
        int size = getTypeSize(node->resolved_type.get());
        if (node->resolved_type && node->resolved_type->category == TypeNode::TypeCategory::ARRAY) {
            std::string qword_reg_name = (reg == RegisterAllocator::RegID::NONE) ? "rax" : allocator.get_name(reg, 8);
            out << "    lea " << qword_reg_name << ", [rel " << asm_label << "]" << std::endl;
        } else {
            if (is_lvalue) {
                std::string qword_reg_name = (reg == RegisterAllocator::RegID::NONE) ? "rax" : allocator.get_name(reg, 8);
                out << "    lea " << qword_reg_name << ", [rel " << asm_label << "]" << std::endl;
            } else {
                if (is_float || is_double) {
                    std::string instr = is_float ? "vmovss" : "vmovsd";
                    out << "    " << instr << " xmm0, [rel " << asm_label << "]" << std::endl;
                } else {
                    if (size == 1) {
                        out << "    movsx " << reg_name << ", byte [rel " << asm_label << "]" << std::endl;
                    } else if (size == 4) {
                        out << "    movsx " << reg_name << ", dword [rel " << asm_label << "]" << std::endl;
                    } else {
                        emit_mov_global(reg_name, asm_label, size);
                    }
                }
            }
        }
        return reg;
    }
    if (is_lvalue) {
        std::string qword_reg_name = allocator.get_name(reg, 8);
        emit("lea", qword_reg_name, "[rbp + " + std::to_string(offset) + "]");
    } else {
        load_adv(node->resolved_type, (is_float || is_double) ? "xmm0" : reg_name, "rbp", offset);
    }

    return reg;
}


RegisterAllocator::RegID CodeGenerator::visit(BinaryOperationExpressionNode* node) {
    bool is_float = false;
    bool is_double = false;

    // Intercept operand types instead of result types to handle comparisons (e.g., radius > 5.0)
    if (node->left && node->left->resolved_type && 
        node->left->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE) {

        auto prim = std::static_pointer_cast<PrimitiveTypeNode>(node->left->resolved_type);
        is_float = (prim->primitive_type == Token::KEYWORD_FLOAT ||
                    prim->primitive_type == Token::FLOAT_LITERAL);
        is_double = (prim->primitive_type == Token::KEYWORD_DOUBLE ||
                     prim->primitive_type == Token::DOUBLE_LITERAL);
    }

    if (is_float || is_double) {
        RegisterAllocator::RegID left_leak = evaluate_expression(node->left.get());
        if (left_leak != RegisterAllocator::RegID::NONE) {
            allocator.free_reg(left_leak);
        }

        out << "    sub rsp, 8" << std::endl;
        current_stack_depth += 8;
        if (is_double) {
            out << "    vmovsd qword [rsp], xmm0" << std::endl;
        } else {
            out << "    vmovss dword [rsp], xmm0" << std::endl;
        }

        RegisterAllocator::RegID right_leak = evaluate_expression(node->right.get());
        if (right_leak != RegisterAllocator::RegID::NONE) {
            allocator.free_reg(right_leak);
        }

        if (is_double) {
            out << "    vmovsd xmm1, qword [rsp]" << std::endl;
        } else {
            out << "    vmovss xmm1, dword [rsp]" << std::endl;
        }
        out << "    add rsp, 8" << std::endl;
        current_stack_depth -= 8;

        char type = is_float ? 'f' : 'l';
        switch (node->op_type) {
            case Token::PLUS:  emit_binary_op("add", type); break;
            case Token::MINUS: emit_binary_op("sub", type); break;
            case Token::STAR:  emit_binary_op("imul", type); break;
            case Token::SLASH: emit_binary_op("idiv", type); break;
            // Handle floating-point comparisons
            case Token::GREATER:
                if (is_double) emit("vcomisd", "xmm1", "xmm0");
                else emit("vcomiss", "xmm1", "xmm0");
                emit("seta", "al");
                emit("movzx", "eax", "al");
                break;
            case Token::LESS:
                if (is_double) emit("vcomisd", "xmm0", "xmm1");
                else emit("vcomiss", "xmm0", "xmm1");
                emit("seta", "al");
                emit("movzx", "eax", "al");
                break;
            default: 
                throw std::runtime_error("Unsupported or unhandled floating point binary operator.");
        }

        return RegisterAllocator::RegID::NONE;
    }

    if (allocator.is_empty()) {
        RegisterAllocator::RegID left_reg = evaluate_expression(node->left.get());
        std::string lhs = (left_reg == RegisterAllocator::RegID::NONE) ? "rax" : reg_to_str(left_reg, node->resolved_type.get());
        if (left_reg != RegisterAllocator::RegID::NONE) {
            out << "    mov rax, " << lhs << std::endl;
            allocator.free_reg(left_reg);
        }
        out << "    push rax" << std::endl; 
        current_stack_depth += 8;

        RegisterAllocator::RegID right_reg = evaluate_expression(node->right.get());
        std::string rhs = (right_reg == RegisterAllocator::RegID::NONE) ? "rax" : reg_to_str(right_reg, node->resolved_type.get());

        out << "    mov r11, " << rhs << std::endl;
        if (right_reg != RegisterAllocator::RegID::NONE) {
            allocator.free_reg(right_reg);
        }

        out << "    pop rax" << std::endl;
        current_stack_depth -= 8;

        std::string target_rax = (getTypeSize(node->resolved_type.get()) == 4) ? "eax" : "rax";
        std::string target_r11 = (getTypeSize(node->resolved_type.get()) == 4) ? "r11d" : "r11";

        switch (node->op_type) {
            case Token::PLUS:  out << "    add " << target_rax << ", " << target_r11 << std::endl; break;
            case Token::MINUS: out << "    sub " << target_rax << ", " << target_r11 << std::endl; break;
            case Token::STAR:  out << "    imul " << target_rax << ", " << target_r11 << std::endl; break;
            case Token::SLASH: {
                std::string rdx_name = (getTypeSize(node->resolved_type.get()) == 4) ? "edx" : "rdx";
                if (getTypeSize(node->resolved_type.get()) == 4) out << "    cdq" << std::endl;
                else out << "    cqo" << std::endl;
                out << "    idiv " << target_r11 << std::endl;
                break;
            }
            case Token::EQUAL_EQUAL:
                out << "    cmp " << target_rax << ", " << target_r11 << std::endl;
                out << "    sete al\n    movzx " << target_rax << ", al" << std::endl;
                break;
            case Token::BANG_EQUAL:
                out << "    cmp " << target_rax << ", " << target_r11 << std::endl;
                out << "    setne al\n    movzx " << target_rax << ", al" << std::endl;
                break;
            case Token::LESS:
                out << "    cmp " << target_rax << ", " << target_r11 << std::endl;
                out << "    setl al\n    movzx " << target_rax << ", al" << std::endl;
                break;
            case Token::GREATER:
                out << "    cmp " << target_rax << ", " << target_r11 << std::endl;
                out << "    setg al\n    movzx " << target_rax << ", al" << std::endl;
                break;
            default: throw std::runtime_error("Unknown stack-allocated binary operator.");
        }

        return RegisterAllocator::RegID::NONE;
    }

    // STANDARD HARDWARE REGISTER ALLOCATION PATH
    RegisterAllocator::RegID left_reg = evaluate_expression(node->left.get());
    RegisterAllocator::RegID right_reg = evaluate_expression(node->right.get());

    std::string lhs = (left_reg == RegisterAllocator::RegID::NONE) ? "rax" : reg_to_str(left_reg, node->resolved_type.get());
    std::string rhs = (right_reg == RegisterAllocator::RegID::NONE) ? "rax" : reg_to_str(right_reg, node->resolved_type.get());

    switch (node->op_type) {
        case Token::PLUS:
            emit("add", lhs, rhs);
            break;
        case Token::MINUS:
            emit("sub", lhs, rhs);
            break;
        case Token::STAR:
            emit("imul", lhs, rhs);
            break;
        case Token::SLASH: {
            std::string rax_name = (getTypeSize(node->resolved_type.get()) == 4) ? "eax" : "rax";
            std::string rdx_name = (getTypeSize(node->resolved_type.get()) == 4) ? "edx" : "rdx";
            
            emit("mov", rax_name, lhs);
            if (getTypeSize(node->resolved_type.get()) == 4) emit("cdq");
            else emit("cqo");
            
            emit("idiv", rhs);
            emit("mov", lhs, rax_name);
            break;
        }
        case Token::EQUAL_EQUAL:
            if (node->left->resolved_type &&
                node->left->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE &&
                static_cast<PrimitiveTypeNode*>(node->left->resolved_type.get())->primitive_type == Token::KEYWORD_STRING) {
                emit("mov", "rdi", lhs);
                emit("mov", "rsi", rhs);
                emit("call", "strcmp");
                emit("test", "rax", "rax");
                emit("sete", "al");
                emit("movzx", lhs, "al");
            } else {
                emit("cmp", lhs, rhs);
                emit("sete", "al");
                emit("movzx", lhs, "al");
            }
            break;
        case Token::BANG_EQUAL:
            if (node->left->resolved_type &&
                node->left->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE &&
                static_cast<PrimitiveTypeNode*>(node->left->resolved_type.get())->primitive_type == Token::KEYWORD_STRING) {
                emit("mov", "rdi", lhs);
                emit("mov", "rsi", rhs);
                emit("call", "strcmp");
                emit("test", "rax", "rax");
                emit("setne", "al");
                emit("movzx", lhs, "al");
            } else {
                emit("cmp", lhs, rhs);
                emit("setne", "al");
                emit("movzx", lhs, "al");
            }
            break;
        case Token::LESS:
            emit("cmp", lhs, rhs);
            emit("setl", "al");
            emit("movzx", lhs, "al");
            break;
        case Token::GREATER:
            emit("cmp", lhs, rhs);
            emit("setg", "al");
            emit("movzx", lhs, "al");
            break;
        case Token::LESS_EQUAL:
            emit("cmp", lhs, rhs);
            emit("setle", "al");
            emit("movzx", lhs, "al");
            break;
        case Token::GREATER_EQUAL:
            emit("cmp", lhs, rhs);
            emit("setge", "al");
            emit("movzx", lhs, "al");
            break;
        default:
            throw std::runtime_error("Unknown binary operator.");
    }

    if (right_reg != RegisterAllocator::RegID::NONE) allocator.free_reg(right_reg);

    return (left_reg == RegisterAllocator::RegID::NONE) ? RegisterAllocator::RegID::NONE : left_reg;
}

RegisterAllocator::RegID CodeGenerator::evaluate_expression(ASTNode* node) {
    if (node == nullptr) return RegisterAllocator::RegID::NONE;
    if (auto integer_node = dynamic_cast<IntegerLiteralExpressionNode*>(node)) {
        return visit(integer_node);
    }
    if (auto var_ref_node = dynamic_cast<VariableReferenceNode*>(node)) {
        return visit(var_ref_node);
    }
    if (auto bin_op_node = dynamic_cast<BinaryOperationExpressionNode*>(node)) {
        return visit(bin_op_node);
    }
    visit(node); 
    return RegisterAllocator::RegID::NONE;
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
        auto* var_ref = dynamic_cast<VariableReferenceNode*>(node->expression.get());
        if (var_ref && var_ref->resolved_symbol && var_ref->resolved_symbol->offset < 0) {
            int size = getTypeSize(var_ref->resolved_type.get());
            int offset = var_ref->resolved_symbol->offset;

            if (size == 1) {
                out << "    movsx rax, byte [rbp + " << offset << "]" << std::endl;
            } else if (size == 4) {
                out << "    movsx rax, dword [rbp + " << offset << "]" << std::endl;
            } else {
                out << "    mov rax, [rbp + " << offset << "]" << std::endl;
            }
        } else {
            visit(node->expression.get());
        }
        if (!node->resolved_type) {
            throw std::runtime_error("CodeGen Error: Return statement has an expression but no resolved type.");
        }
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

void CodeGenerator::visit(SwitchStatementNode* node) {  // TODO:
    if (node->use_jump_table) {
        std::cout << "TODO: Generate an actual jump table for switch statement" << std::endl;
    } else {
        std::cout << "TODO: Generate a comparasion chain for switch statement" << std::endl;
        // normal comp chain
    }
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

    if (node->resolved_symbol == nullptr) {
        throw std::runtime_error("CodeGen Error: Function " + node->function_name + " not found.");
    }

    for (int i = arg_count - 1; i >= (int)arg_regs_64.size(); --i) {
        visit(node->arguments[i].get());
        out << "    push rax" << std::endl;
        current_stack_depth += 8;
    }

    for (int i = std::min(arg_count, (int)arg_regs_64.size()) - 1; i >= 0; --i) {
        visit(node->arguments[i].get());

        int size = 8;
        if (node->arguments[i]->resolved_type) {
            size = getTypeSize(node->arguments[i]->resolved_type.get());
        } else {
            std::cerr << "Warning: Argument " << i << " in call to '" << node->function_name
                      << "' has no resolved type. Defaulting to 8 bytes." << std::endl;
        }

        if (size == 8) {
            out << "    mov " << arg_regs_64[i] << ", rax" << std::endl;
        } else {
            out << "    mov " << arg_regs_32[i] << ", eax" << std::endl;
        }
    }

    std::string target_label = node->resolved_symbol->mangled_name;

    out << "    call " << target_label << std::endl;

    if (arg_count > arg_regs_64.size()) {
        int cleanup = (arg_count - arg_regs_64.size()) * 8;
        out << "    add rsp, " << cleanup << std::endl;
        current_stack_depth -= cleanup;
    }
}

void CodeGenerator::visit(MemberAccessNode* node) {
    bool old_lvalue = is_lvalue;
    is_lvalue = true;
    visit(node->struct_expr.get());
    is_lvalue = old_lvalue;  // Restore state

    Symbol* member_symbol = node->resolved_symbol;
    if ((member_symbol != nullptr) && member_symbol->offset != 0) {
        out << "    add rax, " << member_symbol->offset << std::endl;
    }

    if (!node->resolved_type) {
        throw std::runtime_error("CodeGen Error: Member access '" + node->member_name +
                                 "' has no resolved type.");
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
}

void CodeGenerator::visit(UnaryOpExpressionNode* node) {
    if (node->op_type == Token::KEYWORD_INT || node->op_type == Token::KEYWORD_CHAR) {
        visit(node->operand.get());
        if (isFloatingPoint(node->operand->resolved_type)) {
            int size = getTypeSize(node->operand->resolved_type.get());
            if (size == 8) {
                emit("cvtsd2si", "rax", "xmm0");
            } else {
                emit("cvtss2si", "rax", "xmm0");
            }
        }
        // should sucessfully get the address on its own if its truncation/extension
        return;
    }

    if (node->op_type == Token::ADDRESSOF) {
        const auto* ref_node = static_cast<const VariableReferenceNode*>(node->operand.get());
        Symbol* var_symbol = ref_node->resolved_symbol;
        if (var_symbol == nullptr) {
            throw std::runtime_error(
                "Code generation error: variable '" + ref_node->name +
                "' used before declaration for address-of (resolved_symbol is null).");
        }
        int offset = var_symbol->offset;
        out << "    lea rax, [rbp + " << std::to_string(offset) << "]" << std::endl;
        return;
    }

    visit(node->operand.get());
    if (node->op_type == Token::STAR) {
        out << "    mov rax, [rax]" << std::endl;
    } else if (node->op_type == Token::BANG) {
        //visit(node->operand.get());
        out << "    test rax, rax" << std::endl;
        out << "    setz al" << std::endl;
        out << "    movzx rax, al" << std::endl;
    }
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    bool was_lvalue = is_lvalue;

    is_lvalue = false;
    visit(node->index_expr.get());

    out << "    mov r11, rax" << std::endl;

    int element_size = 8;
    if (node->array_expr && node->array_expr->resolved_type) {
        if (node->array_expr->resolved_type->category == TypeNode::TypeCategory::ARRAY) {
            auto arr_type = static_cast<ArrayTypeNode*>(node->array_expr->resolved_type.get());
            element_size = getTypeSize(arr_type->base_type.get());
        } else if (node->array_expr->resolved_type->category == TypeNode::TypeCategory::POINTER) {
            auto ptr_type = static_cast<PointerTypeNode*>(node->array_expr->resolved_type.get());
            element_size = getTypeSize(ptr_type->base_type.get());
        }
    }

    out << "    imul r11, " << element_size << std::endl;

    is_lvalue = was_lvalue;
    visit(node->array_expr.get());

    out << "    add rax, r11" << std::endl;

    if (!was_lvalue) {
        if (element_size == 1) {
            out << "    movsx rax, byte [rax]" << std::endl;
        } else if (element_size == 4) {
            out << "    movsx rax, dword [rax]" << std::endl;
        } else {
            out << "    mov rax, [rax]" << std::endl;
        }
    }
}

void CodeGenerator::visit(StructDefinitionNode* node) {
    // No code generation needed for struct definitions
}

RegisterAllocator::RegID CodeGenerator::visit(IntegerLiteralExpressionNode* node) {
    RegisterAllocator::RegID reg = allocator.allocate();
    std::string reg_name = reg_to_str(reg, node->resolved_type.get());
    emit("mov", reg_name, std::to_string(node->value));
    return reg;
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

    out << "    vmovsd xmm0, " << "qword [rel " << constants_map[val_str] << "]" << std::endl;
}

void CodeGenerator::visit(StringLiteralExpressionNode* node) {
    std::string formatted_val = "\"" + unescapeString(node->value) + "\", 0";
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
    if (!node->resolved_type) {
        node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
    }
}

void CodeGenerator::visit(AsmStatementNode* node) {
    for (const auto& line : node->lines) {
        out << "    " << line << "\n";
    }
}

int CodeGenerator::getTypeSize(const TypeNode* type) {
    if (type == nullptr) {
        std::cerr << "Type is null" << std::endl;
        throw std::runtime_error("Code Generation Error: Attempted to get size of a null type.");
    }

    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const PrimitiveTypeNode* prim_type = static_cast<const PrimitiveTypeNode*>(type);
            switch (prim_type->primitive_type) {
                case Token::KEYWORD_INT:
                    return 4;
                case Token::KEYWORD_BOOL:
                case Token::KEYWORD_CHAR:
                    return 1;
                case Token::KEYWORD_STRING:
                    return 8;
                case Token::KEYWORD_VOID:
                    return 0;
                case Token::KEYWORD_FLOAT:
                    return 4;
                case Token::KEYWORD_DOUBLE:
                    return 8;
                default:
                    throw std::runtime_error("Code Generation Error: Unknown primitive type (" +
                                             std::to_string((int)prim_type->primitive_type) +
                                             ") for size calculation. (Type category: " +
                                             std::to_string((int)type->category) + ")");
            }
        }
        case TypeNode::TypeCategory::POINTER:
            return 8;
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
            if ((struct_def_symbol == nullptr) || !struct_def_symbol->structDef) {
                const auto& structs = symbolTable.getStructDefinitions();
                if (structs.count(struct_type->struct_name) != 0u) {
                    auto struct_ptr = structs.at(struct_type->struct_name);
                    if (struct_ptr != nullptr) {
                        return struct_ptr->size;
                    }
                    std::cout << "CRITICAL: Struct '" << struct_type->struct_name
                              << "' exists in registry but pointer is NULL!" << std::endl;
                }
                throw std::runtime_error("Code Generation Error: Undefined struct '" +
                                         struct_type->struct_name + "'.");
            }
            return struct_def_symbol->structDef->size;
        }
        default:
            throw std::runtime_error(
                "Code Generation Error: Unknown type category for size calculation.");
    }
}
