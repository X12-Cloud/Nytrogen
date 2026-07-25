#include "code_generator.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

#include "instruction_set.hpp"

CodeGenerator::CodeGenerator(std::unique_ptr<ProgramNode>& ast,
                             SymbolTable& symTable)
    : program_ast(ast),
      symbolTable(symTable),
      string_label_counter(0),
      emitter(out) {}

std::string unescapeString(const std::string& input) {
    std::string result;
    for (size_t i = 0; i < input.size(); ++i) {
        if (input[i] == '\\' && i + 1 < input.size()) {
            switch (input[i + 1]) {
                case 'n':
                    result += "\", 10, \"";
                    break;
                case 't':
                    result += "\", 9, \"";
                    break;
                case 'r':
                    result += "\", 13, \"";
                    break;
                case '"':
                    result += "\", 34, \"";
                    break;
                case '\\':
                    result += "\", 92, \"";
                    break;
                case '0':
                    if (i + 3 < input.size() &&
                        input.substr(i + 1, 3) == "033") {
                        result += "\", 27, \"[";
                        i += 3;
                    } else {
                        result += input[i];
                        result += input[i + 1];
                    }
                    break;
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

void CodeGenerator::generate(const std::string& output_filename,
                             bool is_entry_point) {
    out.open(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file: " +
                                 output_filename);
    }

    // print formats
    constants.push_back({"_print_int_format", "db", "\"%d\", 10, 0"});
    constants.push_back({"_print_str_format", "db", "\"%s\", 10, 0"});
    constants.push_back({"_print_char_format", "db", "\"%c\", 10, 0"});
    constants.push_back({"_print_float_format", "db", "\"%f\", 10, 0"});
    constants.push_back({"_print_int_raw_format", "db", "\"%d\", 0"});
    constants.push_back({"_print_raw_format", "db", "\"%s\", 0"});
    constants.push_back({"_print_char_raw_format", "db", "\"%c\", 0"});
    constants.push_back({"_print_float_raw_format", "db", "\"%f\", 0"});
    constants.push_back({"align", "16"});
    constants.push_back({"_abs_mask", "dq", "0x7FFFFFFFFFFFFFFF"});
    constants.push_back({"align", "8"});

    emitter.section(".text");
    emitter.extern_sym("printf");
    emitter.extern_sym("strcmp");

    if (is_entry_point) {
        emitter.global("_start");
    }

    for (const auto& func : program_ast->functions) {
        if (!func->body_statements.empty()) {
            emitter.global(func->name);
        } else {
            emitter.extern_sym(func->name);
        }
    }

    // entry point
    if (is_entry_point) {
        emitter.label("_start");
        emitter.emit("call", "main");
        emitter.emit("mov", "rdi", "rax");
        emitter.syscall(60);
    }

    visit(program_ast.get());

    // print the .data section
    emitter.section(".data");
    std::unordered_set<std::string> emitted_data_labels;
    for (const auto& c : constants) {
        if (c.label == "align") {
            emitter.emit("align", c.type);
            continue;
        }
        if (emitted_data_labels.find(c.label) == emitted_data_labels.end()) {
            emitter.emit_data_entry(c.label, c.type, c.value);
            emitted_data_labels.insert(c.label);
        }
    }

    emitter.flush_to_file();

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
            throw std::runtime_error(
                "Code Generation Error: Unknown AST node type.");
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
        emitter.extern_sym(node->mangled_name);
        return;  // No further code generation for extern functions
    }

    emitter.label_local(node->mangled_name);
    emitter.emit("push", "rbp");
    current_stack_depth += 8;
    emitter.emit("mov", "rbp", "rsp");

    emitter.emit("and", "rsp", "-16");
    current_stack_depth = 0;

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
        emitter.emit("sub", "rsp", std::to_string(aligned_space));
        current_stack_depth += aligned_space;
    }

    // Map incoming argument registers into virtual registers
    const std::vector<std::string> arg_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (size_t i = 0; i < node->parameters.size() && i < arg_registers.size(); ++i) {
        Symbol* sym = symbolTable.lookup(node->parameters[i]->name);
        if (sym) {
            std::string param_vreg = vreg_lookup(sym);

            emitter.emit("mov", param_vreg, arg_registers[i]);

            int offset = (i + 1) * -8;
            emitter.mov_indirect("rbp", offset, arg_registers[i]);
        }
    }

    // Generate code for all statements
    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    emitter.label_local(current_function_name + "_epilogue");

    emitter.emit("leave");
    emitter.emit("ret");

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
        auto it =
            ns_symbol->internal_scope->symbols.find(node->member->get_value());
        if (it != ns_symbol->internal_scope->symbols.end()) {
            Symbol& sym = it->second;
            if (sym.dataType) {
                node->member->resolved_type = sym.dataType->clone();
                if (node->member->node_type ==
                    ASTNode::NodeType::VARIABLE_REFERENCE) {
                    auto* var =
                        static_cast<VariableReferenceNode*>(node->member.get());
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
    bool is_float =
        (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT);
    bool is_double =
        (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_string =
        (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_STRING);
    int size = getTypeSize(node->type.get());
    std::string asm_label;
    for (auto& decl : node->declarations) {
        Symbol* symbol = decl.resolved_symbol;
        if (symbol == nullptr) {
            throw std::runtime_error("Code generation error: variable '" +
                                     decl.name +
                                     "' not found in symbol table.");
        }

        // Fetch or assign the virtual register for this declared variable
        std::string vreg = vreg_lookup(decl.resolved_symbol);

        std::string final_name = symbol->mangled_name;

        if (!final_name.empty() && final_name != decl.name) {
            std::string init_val = "0";
            bool has_non_const_init = false;

            if (decl.initial_value) {
                if (decl.initial_value->is_constant()) {
                    if (is_string) {
                        std::string str_data_label =
                            "_str_var_data_" +
                            std::to_string(string_label_counter++);
                        constants.push_back(
                            {str_data_label, "db",
                             "\"" +
                                 unescapeString(
                                     decl.initial_value->get_value()) +
                                 "\", 0"});
                        init_val = str_data_label;
                    } else {
                        init_val = decl.initial_value->get_value();
                    }
                } else {
                    has_non_const_init = true;
                }
            }

            std::string nasm_type = (size == 4)                ? "dd"
                                    : (is_string || size == 8) ? "dq"
                                    : (size == 1)              ? "db"
                                                               : "dw";

            constants.push_back({final_name, nasm_type, init_val});

            if (has_non_const_init) {
                visit(decl.initial_value.get());
                if (is_float || is_double) {
                    std::string instr = is_float ? "vmovss" : "vmovsd";
                    emitter.emit_mem_rel(instr, final_name, last_expr_vreg);
                } else {
                    emitter.emit_mem_rel("mov", final_name, last_expr_vreg);
                }
            }
        } else {
            bool is_fp = is_double || is_float;
            if (decl.initial_value) {
                visit(decl.initial_value.get());
                emitter.emit(is_fp ? "vmovsd" : "mov", vreg, last_expr_vreg);
            }
        }
    }
}

void CodeGenerator::visit(VariableAssignmentNode* node) {
    auto type = node->left->resolved_type;
    bool is_fp = isFloatingPoint(type.get());

    visit(node->right.get());
    std::string rhs_vreg = last_expr_vreg;

    auto* var_ref = dynamic_cast<VariableReferenceNode*>(node->left.get());
    if (var_ref != nullptr) {
        std::string lhs_vreg = vreg_lookup(var_ref->resolved_symbol);

        if (is_fp) {
            std::string instr = (getTypeSize(type.get()) == 4) ? "vmovss" : "vmovsd";
            emitter.emit(instr, lhs_vreg, rhs_vreg);
        } else {
            emitter.emit("mov", lhs_vreg, rhs_vreg);
        }
        last_expr_vreg = lhs_vreg;
    } else {
        // Complex lvalue (e.g., array access or member access)
        is_lvalue = true;
        visit(node->left.get());
        is_lvalue = false;
        std::string addr_vreg = last_expr_vreg;

        int size = getTypeSize(type.get());
        emitter.emit_adv(size, type.get(), addr_vreg, 0, rhs_vreg);
        last_expr_vreg = rhs_vreg;
    }
}

void CodeGenerator::visit(VariableReferenceNode* node) {
    Symbol* symbol = node->resolved_symbol;

    if (symbol == nullptr) {
        throw std::runtime_error("CodeGen Error: Symbol not resolved for " + node->name);
    }

    if (symbol->type == Symbol::SymbolType::CONSTANT) {
        visit(symbol->value.get());
        return;
    }

    std::string vreg = vreg_lookup(symbol);

    bool is_global = !symbol->mangled_name.empty() && symbol->mangled_name != symbol->name;
    if (is_global && !is_lvalue) {
        int size = getTypeSize(node->resolved_type.get());
        bool is_fp = isFloatingPoint(node->resolved_type.get());

        if (is_fp) {
            std::string instr = (size == 4) ? "vmovss" : "vmovsd";
            emitter.emit(instr, vreg, "[rel " + symbol->mangled_name + "]");
        } else {
            // Use movsx for smaller types when loading into 64-bit vregs
            std::string instr = (size == 1) ? "movsx" : (size == 4 ? "movsx" : "mov");
            emitter.emit(instr, vreg, "[rel " + symbol->mangled_name + "]");
        }
    }

    last_expr_vreg = vreg;
}

void CodeGenerator::visit(BinaryOperationExpressionNode* node) {
    bool is_fp = isFloatingPoint(node->resolved_type.get());
    bool is_double = false;
    if (is_fp) {
        auto prim = std::static_pointer_cast<PrimitiveTypeNode>(node->resolved_type);
        is_double = (prim->primitive_type == Token::KEYWORD_DOUBLE || prim->primitive_type == Token::DOUBLE_LITERAL);
    }

    visit(node->left.get());
    std::string left_vreg = last_expr_vreg;

    visit(node->right.get());
    std::string right_vreg = last_expr_vreg;

    std::string result_vreg = new_vreg();

    switch (node->op_type) {
        case Token::PLUS:
            if (is_fp) {
                emitter.emit(is_double ? "vaddsd" : "vaddss", result_vreg, left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", result_vreg, left_vreg);
                emitter.emit("add", result_vreg, right_vreg);
            }
            break;
        case Token::MINUS:
            if (is_fp) {
                emitter.emit(is_double ? "vsubsd" : "vsubss", result_vreg, left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", result_vreg, left_vreg);
                emitter.emit("sub", result_vreg, right_vreg);
            }
            break;
        case Token::STAR:
            if (is_fp) {
                emitter.emit(is_double ? "vmulsd" : "vmulss", result_vreg, left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", result_vreg, left_vreg);
                emitter.emit("imul", result_vreg, right_vreg);
            }
            break;
        case Token::SLASH:
            if (is_fp) {
                emitter.emit(is_double ? "vdivsd" : "vdivss", result_vreg, left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", "rax", left_vreg);
                emitter.emit("cqo");
                emitter.emit("idiv", right_vreg);
                emitter.emit("mov", result_vreg, "rax");
            }
            break;
        default:
            emitter.emit("cmp", left_vreg, right_vreg);
            std::string set_instr;
            if (node->op_type == Token::EQUAL_EQUAL) set_instr = "sete";
            else if (node->op_type == Token::BANG_EQUAL) set_instr = "setne";
            else if (node->op_type == Token::LESS) set_instr = "setl";
            else if (node->op_type == Token::GREATER) set_instr = "setg";

            emitter.emit(set_instr, "al");
            emitter.emit("movzx", result_vreg, "al");
            break;
    }

    last_expr_vreg = result_vreg;
}

void CodeGenerator::visit(PrintStatementNode* node) {
    for (size_t i = 0; i < node->expressions.size(); ++i) {
        visit(node->expressions[i].get());

        auto expr_type = node->expressions[i]->resolved_type;
        int size = getTypeSize(expr_type.get());

        // Move vreg to physical regs for printf (handled inside emitter.emit_print)
        if (i == node->expressions.size() - 1) {
            emitter.emit_print(size, expr_type, last_expr_vreg);
        } else {
            emitter.emit_print_raw(size, expr_type, last_expr_vreg);
        }
    }
}

void CodeGenerator::visit(ReturnStatementNode* node) {
    if (node->expression) {
        visit(node->expression.get());
        bool is_fp = isFloatingPoint(node->expression->resolved_type.get());

        emitter.emit(is_fp ? "vmovsd" : "mov", is_fp ? "xmm0" : "rax", last_expr_vreg);
    }
    emitter.emit("jmp", current_function_name + "_epilogue");
}

void CodeGenerator::visit(IfStatementNode* node) {
    static int if_counter = 0;
    int id = if_counter++;
    std::string false_label = "_if_false_" + std::to_string(id);
    std::string end_label = "_if_end_" + std::to_string(id);

    // Condition
    visit(node->condition.get());
    emitter.emit("cmp", last_expr_vreg, "0");
    emitter.emit("je", false_label);

    // True block
    for (const auto& stmt : node->true_block) visit(stmt.get());
    emitter.emit("jmp", end_label);

    // False block
    emitter.label(false_label);
    for (const auto& stmt : node->false_block) visit(stmt.get());

    emitter.label(end_label);
}

void CodeGenerator::visit(SwitchStatementNode* node) {  // TODO:
    if (node->use_jump_table) {
        std::cout << "TODO: Generate an actual jump table for switch statement"
                  << std::endl;
    } else {
        std::cout << "TODO: Generate a comparasion chain for switch statement"
                  << std::endl;
        // normal comp chain
    }
}

void CodeGenerator::visit(WhileStatementNode* node) {
    static int while_counter = 0;
    int id = while_counter++;
    std::string start_label = "_while_start_" + std::to_string(id);
    std::string end_label = "_while_end_" + std::to_string(id);

    emitter.label(start_label);

    visit(node->condition.get());
    emitter.emit("cmp", last_expr_vreg, "0");
    emitter.emit("je", end_label);

    for (const auto& stmt : node->body) visit(stmt.get());

    emitter.emit("jmp", start_label);
    emitter.label(end_label);
}

void CodeGenerator::visit(ForStatementNode* node) {
    static int for_counter = 0;
    int id = for_counter++;

    std::string condition_label = "_for_loop_condition_" + std::to_string(id);
    std::string start_label = "_for_loop_start_" + std::to_string(id);
    std::string end_label = "_for_loop_end_" + std::to_string(id);

    if (node->initializer) {
        visit(node->initializer.get());
    }

    emitter.label(condition_label);
    if (node->condition) {
        visit(node->condition.get());
        emitter.emit("cmp", last_expr_vreg, "0"); 
        emitter.emit("je", end_label);
    }

    emitter.label(start_label);
    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }

    if (node->increment) {
        visit(node->increment.get());
    }

    emitter.emit("jmp", condition_label);
    emitter.label(end_label);
}

void CodeGenerator::visit(FunctionCallNode* node) {
    const std::vector<std::string> arg_regs = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    std::vector<std::string> evaluated_vregs;

    for (auto& arg : node->arguments) {
        visit(arg.get());
        evaluated_vregs.push_back(last_expr_vreg);
    }

    int int_idx = 0;
    int xmm_idx = 0;
    for (size_t i = 0; i < evaluated_vregs.size(); ++i) {
        bool is_fp = isFloatingPoint(node->arguments[i]->resolved_type.get());
        if (is_fp && xmm_idx < 8) {
            emitter.emit("vmovsd", "xmm" + std::to_string(xmm_idx++), evaluated_vregs[i]);
        } else if (!is_fp && int_idx < 6) {
            emitter.emit("mov", arg_regs[int_idx++], evaluated_vregs[i]);
        }
        // Stack-based arguments would be handled here
    }

    emitter.emit("call", node->resolved_symbol->mangled_name);

    std::string result_vreg = new_vreg();
    bool ret_fp = isFloatingPoint(node->resolved_type.get());
    if (ret_fp) {
        emitter.emit("vmovsd", result_vreg, "xmm0");
    } else {
        emitter.emit("mov", result_vreg, "rax");
    }

    last_expr_vreg = result_vreg;
}

void CodeGenerator::visit(MemberAccessNode* node) {
    bool old_lvalue = is_lvalue;
    is_lvalue = true; 
    visit(node->struct_expr.get());
    std::string base_addr_vreg = last_expr_vreg;
    is_lvalue = old_lvalue;

    std::string member_addr_vreg = new_vreg();
    Symbol* member_symbol = node->resolved_symbol;

    if (member_symbol && member_symbol->offset != 0) {
        emitter.emit("add", member_addr_vreg, base_addr_vreg + ", " + std::to_string(member_symbol->offset));
    } else {
        emitter.emit("mov", member_addr_vreg, base_addr_vreg);
    }

    if (is_lvalue) {
        last_expr_vreg = member_addr_vreg;
    } else {
        std::string val_vreg = new_vreg();
        int size = getTypeSize(node->resolved_type.get());
        emitter.load_adv(size, node->resolved_type.get(), val_vreg, member_addr_vreg, 0);
        last_expr_vreg = val_vreg;
    }
}

void CodeGenerator::visit(UnaryOpExpressionNode* node) {
    if (node->op_type == Token::KEYWORD_INT || node->op_type == Token::KEYWORD_CHAR) {
        visit(node->operand.get());
        std::string res_vreg = new_vreg();
        if (isFloatingPoint(node->operand->resolved_type.get())) {
            int size = getTypeSize(node->operand->resolved_type.get());
            emitter.emit(size == 8 ? "vcvttsd2si" : "vcvttss2si", res_vreg, last_expr_vreg);
        } else {
            emitter.emit("mov", res_vreg, last_expr_vreg); // Identity cast
        }
        last_expr_vreg = res_vreg;
        return;
    }

    if (node->op_type == Token::ADDRESSOF) {
        is_lvalue = true;
        visit(node->operand.get());
        is_lvalue = false;
        // last_expr_vreg already contains the address from the child's lvalue logic
        return;
    }

    visit(node->operand.get());
    std::string op_vreg = last_expr_vreg;
    std::string res_vreg = new_vreg();

    if (node->op_type == Token::STAR) { // Dereference
        int size = getTypeSize(node->resolved_type.get());
        emitter.load_adv(size, node->resolved_type.get(), res_vreg, op_vreg, 0);
    } else if (node->op_type == Token::BANG) { // Logical NOT
        emitter.emit("test", op_vreg, op_vreg);
        emitter.emit("setz", "al");
        emitter.emit("movzx", res_vreg, "al");
    }
    last_expr_vreg = res_vreg;
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    bool was_lvalue = is_lvalue;

    is_lvalue = false;
    visit(node->index_expr.get());
    std::string index_vreg = last_expr_vreg;

    std::string base_vreg = new_vreg();
    if (node->array_expr->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
        auto var_ref = static_cast<VariableReferenceNode*>(node->array_expr.get());
        if (var_ref->resolved_symbol) {
            // bridge: Get stack offset address into a virtual register
            emitter.emit_lea_stack(base_vreg, var_ref->resolved_symbol->offset);
        }
    } else {
        visit(node->array_expr.get());
        emitter.emit("mov", base_vreg, last_expr_vreg);
    }
    is_lvalue = was_lvalue;

    int element_size = 8;
    if (node->array_expr->resolved_type && node->array_expr->resolved_type->category == TypeNode::TypeCategory::ARRAY) {
        auto arr_type = static_cast<ArrayTypeNode*>(node->array_expr->resolved_type.get());
        element_size = getTypeSize(arr_type->base_type.get());
    }

    // addr = base + (index * size)
    std::string offset_vreg = new_vreg();
    emitter.emit("imul", offset_vreg, index_vreg + ", " + std::to_string(element_size));

    std::string final_addr_vreg = new_vreg();
    emitter.emit("add", final_addr_vreg, base_vreg + ", " + offset_vreg);

    if (is_lvalue) {
        last_expr_vreg = final_addr_vreg;
    } else {
        std::string val_vreg = new_vreg();
        emitter.load_adv(element_size, node->resolved_type.get(), val_vreg, final_addr_vreg, 0);
        last_expr_vreg = val_vreg;
    }
}

void CodeGenerator::visit(StructDefinitionNode* node) {
    // No code generation needed for struct definitions
}

void CodeGenerator::visit(IntegerLiteralExpressionNode* node) {
    std::string vreg = new_vreg();
    emitter.emit("mov", vreg, std::to_string(node->value));
    last_expr_vreg = vreg;
}

void CodeGenerator::visit(FloatLiteralExpressionNode* node) {
    std::string val_str = std::to_string(node->value);
    if (constants_map.find(val_str) == constants_map.end()) {
        std::string label = "_float_" + std::to_string(string_label_counter++);
        constants_map[val_str] = label;
        constants.push_back({label, "dd", val_str});
    }

    std::string vreg = new_vreg();
    emitter.emit("vmovss", vreg, "[rel " + constants_map[val_str] + "]");
    last_expr_vreg = vreg;
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

    std::string vreg = new_vreg();
    emitter.emit("vmovsd", vreg, "[rel " + constants_map[val_str] + "]");
    last_expr_vreg = vreg;
}

void CodeGenerator::visit(StringLiteralExpressionNode* node) {
    std::string formatted_val = "\"" + unescapeString(node->value) + "\", 0";
    if (constants_map.find(formatted_val) == constants_map.end()) {
        std::string label = "_str_" + std::to_string(string_label_counter++);
        constants_map[formatted_val] = label;
        constants.push_back({label, "db", formatted_val});
    }

    std::string vreg = new_vreg();
    emitter.emit("lea", vreg, "[rel " + constants_map[formatted_val] + "]");
    last_expr_vreg = vreg;
}

void CodeGenerator::visit(BooleanLiteralExpressionNode* node) {
    std::string vreg = new_vreg();
    emitter.emit("mov", vreg, std::to_string(node->value ? 1 : 0));
    last_expr_vreg = vreg;
}

void CodeGenerator::visit(CharacterLiteralExpressionNode* node) {
    std::string vreg = new_vreg();
    emitter.emit("mov", vreg, std::to_string(static_cast<int>(node->value)));
    last_expr_vreg = vreg;
    if (!node->resolved_type) {
        node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_CHAR);
    }
}

void CodeGenerator::visit(AsmStatementNode* node) {
    for (const auto& line : node->lines) {
        emitter.emit(line);
    }
}

int CodeGenerator::getTypeSize(const TypeNode* type) {
    if (type == nullptr) {
        std::cerr << "Type is null" << std::endl;
        throw std::runtime_error(
            "Code Generation Error: Attempted to get size of a null type.");
    }

    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const PrimitiveTypeNode* prim_type =
                static_cast<const PrimitiveTypeNode*>(type);
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
                    throw std::runtime_error(
                        "Code Generation Error: Unknown primitive type (" +
                        std::to_string((int)prim_type->primitive_type) +
                        ") for size calculation. (Type category: " +
                        std::to_string((int)type->category) + ")");
            }
        }
        case TypeNode::TypeCategory::POINTER:
            return 8;
        case TypeNode::TypeCategory::ARRAY: {
            const ArrayTypeNode* array_type =
                static_cast<const ArrayTypeNode*>(type);
            int element_size = getTypeSize(array_type->base_type.get());
            if (array_type->size > 0) {
                return element_size * array_type->size;
            }
            return 0;
        }
        case TypeNode::TypeCategory::STRUCT: {
            const StructTypeNode* struct_type =
                static_cast<const StructTypeNode*>(type);
            Symbol* struct_def_symbol =
                symbolTable.lookup(struct_type->struct_name);
            if ((struct_def_symbol == nullptr) ||
                !struct_def_symbol->structDef) {
                const auto& structs = symbolTable.getStructDefinitions();
                if (structs.count(struct_type->struct_name) != 0u) {
                    auto struct_ptr = structs.at(struct_type->struct_name);
                    if (struct_ptr != nullptr) {
                        return struct_ptr->size;
                    }
                    std::cout << "CRITICAL: Struct '"
                              << struct_type->struct_name
                              << "' exists in registry but pointer is NULL!"
                              << std::endl;
                }
                throw std::runtime_error(
                    "Code Generation Error: Undefined struct '" +
                    struct_type->struct_name + "'.");
            }
            return struct_def_symbol->structDef->size;
        }
        default:
            throw std::runtime_error(
                "Code Generation Error: Unknown type category for size "
                "calculation.");
    }
}

bool CodeGenerator::isFloatingPoint(const TypeNode* type) {
    if (type == nullptr) {
        return false;
    }
    auto prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}

std::string CodeGenerator::getRegisterName(const std::string& reg64, int size) {
    if (size == 8) return reg64;

    static const std::unordered_map<std::string, std::string> map32 = {
        {"rdi", "edi"}, {"rsi", "esi"}, {"rdx", "edx"},
        {"rcx", "ecx"}, {"r8", "r8d"},  {"r9", "r9d"}};

    return map32.at(reg64);
}
