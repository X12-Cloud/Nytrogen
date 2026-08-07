#include "code_generator.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

#include "instruction_set.hpp"

CodeGenerator::CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable)
    : program_ast(ast), symbolTable(symTable), emitter(out) {}

auto unescapeString(const std::string& input) -> std::string {
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
                    if (i + 3 < input.size() && input.substr(i + 1, 3) == "033") {
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

void CodeGenerator::generate(const std::string& output_filename, bool is_entry_point) {
    out.open(output_filename);
    if (!out.is_open()) {
        throw std::runtime_error("Could not open output file: " + output_filename);
    }

    constants.push_back({"align", "16"});
    constants.push_back({"_abs_mask", "dq", "0x7FFFFFFFFFFFFFFF"});
    constants.push_back({"align", "8"});

    emitter.section(".text");

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

        emitter.emit("lea", "rdi", "[rel _N_qlib_state]");
        emitter.call_external("setup");

        emitter.emit("call", "main");
        emitter.emit("mov", "rdi", "rax");
        emitter.call_external("ny_exit");
    }

    visit(program_ast.get());

    // print the .data section
    emitter.section(".data");
    std::unordered_set<std::string> emitted_data_labels;
    emitter.emit("global _N_qlib_state");
    constants.push_back({"_N_qlib_state", "times 8192", "db 0"});
    constants.push_back({"align", "32"});
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

    allocator.allocate_registers(emitter.instructions);
    emitter.flush_to_file();

    out.close();
}

void CodeGenerator::visit(ASTNode* node) {
    if (node == nullptr) {
        return;
    }

    switch (node->node_type) {
        case ASTNode::NodeType::PROGRAM:
            visit(dynamic_cast<ProgramNode*>(node));
            break;
        case ASTNode::NodeType::FUNCTION_DEFINITION:
            visit(dynamic_cast<FunctionDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::NAMESPACE_DEFINITION:
            visit(dynamic_cast<NamespaceDefinition*>(node));
            break;
        case ASTNode::NodeType::SCOPE_RESOLUTION:
            visit(dynamic_cast<ScopeResolutionNode*>(node));
            break;
        case ASTNode::NodeType::VARIABLE_DECLARATION:
            visit(dynamic_cast<VariableDeclarationNode*>(node));
            break;
        case ASTNode::NodeType::VARIABLE_ASSIGNMENT:
            visit(dynamic_cast<VariableAssignmentNode*>(node));
            break;
        case ASTNode::NodeType::VARIABLE_REFERENCE:
            visit(dynamic_cast<VariableReferenceNode*>(node));
            break;
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION:
            visit(dynamic_cast<BinaryOperationExpressionNode*>(node));
            break;
        case ASTNode::NodeType::PRINT_STATEMENT:
            visit(dynamic_cast<PrintStatementNode*>(node));
            break;
        case ASTNode::NodeType::RETURN_STATEMENT:
            visit(dynamic_cast<ReturnStatementNode*>(node));
            break;
        case ASTNode::NodeType::IF_STATEMENT:
            visit(dynamic_cast<IfStatementNode*>(node));
            break;
        case ASTNode::NodeType::WHILE_STATEMENT:
            visit(dynamic_cast<WhileStatementNode*>(node));
            break;
        case ASTNode::NodeType::FOR_STATEMENT:
            visit(dynamic_cast<ForStatementNode*>(node));
            break;
        case ASTNode::NodeType::FUNCTION_CALL:
            visit(dynamic_cast<FunctionCallNode*>(node));
            break;
        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION:
            visit(dynamic_cast<MemberAccessNode*>(node));
            break;
        case ASTNode::NodeType::UNARY_OP_EXPRESSION:
            visit(dynamic_cast<UnaryOpExpressionNode*>(node));
            break;
        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION:
            visit(dynamic_cast<ArrayAccessNode*>(node));
            break;
        case ASTNode::NodeType::STRUCT_DEFINITION:
            visit(dynamic_cast<StructDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION:
            visit(dynamic_cast<IntegerLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION:
            visit(dynamic_cast<StringLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION:
            visit(dynamic_cast<BooleanLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION:
            visit(dynamic_cast<CharacterLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::FLOAT_LITERAL_EXPRESSION:
            visit(dynamic_cast<FloatLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::DOUBLE_LITERAL_EXPRESSION:
            visit(dynamic_cast<DoubleLiteralExpressionNode*>(node));
            break;
        case ASTNode::NodeType::ASM_STATEMENT:
            visit(dynamic_cast<AsmStatementNode*>(node));
            break;
        case ASTNode::NodeType::CONSTANT_DECLARATION:
            visit(dynamic_cast<ConstantDeclarationNode*>(node));
            break;
        case ASTNode::NodeType::ENUM_STATEMENT:
            visit(dynamic_cast<EnumStatementNode*>(node));
            break;
        case ASTNode::NodeType::QUBIT_DEFINITION:
            visit(dynamic_cast<QubitDefinitionNode*>(node));
            break;
        case ASTNode::NodeType::GATE_APPLICATION_OPERATION_EXPRESSION:
            visit(dynamic_cast<GateAppOperationExpressionNode*>(node));
            break;
        case ASTNode::NodeType::COMPLEX_LITERAL_EXPRESSION:
            visit(dynamic_cast<ComplexLiteralExpressionNode*>(node));
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
    if (node->is_extern) {
        emitter.extern_sym(node->mangled_name);
        return;
    }

    emitter.label_local(node->mangled_name);
    emitter.emit("push", "rbp");
    emitter.emit("mov", "rbp", "rsp");
    emitter.emit("and", "rsp", "-16");

    auto* original_scope = symbolTable.current_scope;
    bool found = false;

    Symbol* func_sym = symbolTable.lookup(node->mangled_name);
    if (func_sym == nullptr) {
        func_sym = symbolTable.lookup(node->name);
    }

    if ((func_sym != nullptr) && (func_sym->internal_scope != nullptr)) {
        symbolTable.current_scope = func_sym->internal_scope;
        found = true;
    } else {
        // Search all scopes for a name match if the above doesnt work
        for (auto& s : symbolTable.all_scopes) {
            if (s->scope_name == node->mangled_name || s->scope_name == node->name) {
                symbolTable.current_scope = s.get();
                found = true;
                break;
            }
        }
    }

    if (!found) {
        std::cerr << "CRITICAL ERROR: Scope not found for function: " << node->name << std::endl;
    }

    // Allocate stack space
    int local_var_space = -symbolTable.current_scope->currentOffset;
    if (local_var_space <= 0) {
        local_var_space = 128;
    }
    int aligned_space = (local_var_space + 128 + 15) & ~15;
    emitter.emit("sub", "rsp", std::to_string(aligned_space));

    // Bridge: Map physical args to the correct scope symbols
    const std::vector<std::string> arg_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    for (size_t i = 0; i < node->parameters.size() && i < arg_registers.size(); ++i) {
        std::string p_name = node->parameters[i]->name;

        Symbol* sym = symbolTable.lookup(p_name);

        if (sym != nullptr) {
            std::string param_vreg = vreg_lookup(sym);
            emitter.emit("mov", param_vreg, arg_registers[i]);
            emitter.mov_indirect("rbp", sym->offset, arg_registers[i]);
        } else {
            std::cerr << "BRIDGE ERROR: Parameter '" << p_name << "' not found in scope."
                      << std::endl;
        }
    }

    for (const auto& stmt : node->body_statements) {
        visit(stmt.get());
    }

    emitter.label_local(current_function_name + "_epilogue");
    emitter.emit("leave");
    emitter.emit("ret");

    symbolTable.current_scope = original_scope;
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
                    auto* var = dynamic_cast<VariableReferenceNode*>(node->member.get());
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
    int size = getTypeSize(node->type.get());
    bool is_fp = isFloatingPoint(node->type.get());
    auto* prim = dynamic_cast<PrimitiveTypeNode*>(node->type.get());
    bool is_complex_type = (prim && prim->primitive_type == Token::KEYWORD_COMPLEX);
    bool is_string = (prim && prim->primitive_type == Token::KEYWORD_STRING);

    for (auto& decl : node->declarations) {
        Symbol* symbol = decl.resolved_symbol;
        if (symbol == nullptr) {
            throw std::runtime_error("Code generation error: variable '" + decl.name + "' not found.");
        }

        std::string vreg = vreg_lookup(decl.resolved_symbol);

        if (symbol->is_global) {
            // Add entry to .data section
            std::string init_val = "0";
            if (decl.initial_value && decl.initial_value->is_constant()) {
                if (is_string) {
                    std::string label = "_str_var_" + std::to_string(string_label_counter++);
                    constants.push_back({label, "db", "\"" + unescapeString(decl.initial_value->get_value()) + "\", 0"});
                    init_val = label;
                    std::cout << "Debug mangled name = " << symbol->mangled_name << std::endl;
                } else {
                    init_val = decl.initial_value->get_value();
                }
            }

            if (is_complex_type) {
                constants.push_back({symbol->mangled_name, "dq", "0.0, 0.0"});
            } else {
                std::string nasm_type = (size == 4) ? "dd" : (size == 8 ? "dq" : "db");
                constants.push_back({symbol->mangled_name, nasm_type, init_val});
            }

            // If it has a non-constant initial value, emit code to set it
            if (decl.initial_value) {
                visit(decl.initial_value.get());
                std::string instr = is_complex_type ? "vmovupd" : (is_fp ? (size == 4 ? "vmovss" : "vmovsd") : "mov");
                emitter.emit_mem_rel(instr, symbol->mangled_name, last_expr_vreg);
                std::cout << "Debug mangled name = " << symbol->mangled_name << std::endl;
            }
        } else {
            // local variable (stack)
            if (decl.initial_value) {
                visit(decl.initial_value.get()); // result in last_expr_vreg

                // For complex (16 bytes), we use vmovupd
                std::string instr = is_complex_type ? "vmovupd" : (is_fp ? (size == 4 ? "vmovss" : "vmovsd") : "mov");
                emitter.emit(instr, vreg, last_expr_vreg);

                emitter.emit_adv(size, node->type.get(), "rbp", symbol->offset, vreg);
            }
        }
    }
}

void CodeGenerator::visit(VariableAssignmentNode* node) {
    auto type = node->left->resolved_type;

    visit(node->right.get());
    std::string rhs_val_vreg = last_expr_vreg;

    bool old_lvalue = is_lvalue;
    is_lvalue = true;
    visit(node->left.get()); 
    std::string lhs_addr_vreg = last_expr_vreg; // memory address
    is_lvalue = old_lvalue;

    int size = getTypeSize(type.get());
    emitter.emit_adv(size, type.get(), lhs_addr_vreg, 0, rhs_val_vreg);

    last_expr_vreg = rhs_val_vreg;
}

void CodeGenerator::visit(VariableReferenceNode* node) {
    Symbol* symbol = node->resolved_symbol;
    if (symbol == nullptr) throw std::runtime_error("CodeGen Error: Symbol not resolved");

    if (symbol->type == Symbol::SymbolType::CONSTANT) {
        visit(symbol->value.get());
        return;
    }

    std::string vreg = vreg_lookup(symbol);
    int size = getTypeSize(node->resolved_type.get());

    auto* prim = dynamic_cast<PrimitiveTypeNode*>(node->resolved_type.get());
    bool is_complex_layout = (node->resolved_type->category == TypeNode::TypeCategory::STRUCT ||
                              node->resolved_type->category == TypeNode::TypeCategory::ARRAY ||
                              (prim && prim->primitive_type == Token::KEYWORD_COMPLEX));

    if (symbol->is_global) {
        std::string prefix = emitter.get_size_prefix(size);
        if (is_lvalue || is_complex_layout) {
            emitter.emit("lea", vreg, "[rel " + symbol->mangled_name + "]");
        } else {
            std::string instr = (size == 1) ? "movsx" : (size == 4 ? "movsxd" : "mov");
            if (isFloatingPoint(node->resolved_type.get()))
                instr = (size == 4 ? "vmovss" : "vmovsd");

            emitter.emit(instr, vreg, prefix + " [rel " + symbol->mangled_name + "]");
            std::cout << "Mangled name for " << symbol->name << " = " << symbol->mangled_name << std::endl;
        }
    } else {
        // local
        if (is_lvalue || is_complex_layout) {
            emitter.emit_lea_stack(vreg, symbol->offset);
        } else {
            emitter.load_adv(size, node->resolved_type.get(), vreg, "rbp", symbol->offset);
        }
    }
    last_expr_vreg = vreg;
}

void CodeGenerator::visit(BinaryOperationExpressionNode* node) {
    bool is_fp = isFloatingPoint(node->resolved_type.get());
    bool is_double = false;
    if (is_fp) {
        auto prim = std::static_pointer_cast<PrimitiveTypeNode>(node->resolved_type);
        is_double = (prim->primitive_type == Token::KEYWORD_DOUBLE ||
                     prim->primitive_type == Token::DOUBLE_LITERAL);
    }

    visit(node->left.get());
    std::string left_vreg = last_expr_vreg;

    visit(node->right.get());
    std::string right_vreg = last_expr_vreg;

    std::string result_vreg = new_vreg();

    switch (node->op_type) {
        case Token::PLUS:
            if (is_fp) {
                emitter.emit(is_double ? "vaddsd" : "vaddss", result_vreg,
                             left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", result_vreg, left_vreg);
                emitter.emit("add", result_vreg, right_vreg);
            }
            break;
        case Token::MINUS:
            if (is_fp) {
                emitter.emit(is_double ? "vsubsd" : "vsubss", result_vreg,
                             left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", result_vreg, left_vreg);
                emitter.emit("sub", result_vreg, right_vreg);
            }
            break;
        case Token::STAR:
            if (is_fp) {
                emitter.emit(is_double ? "vmulsd" : "vmulss", result_vreg,
                             left_vreg + ", " + right_vreg);
            } else {
                emitter.emit("mov", result_vreg, left_vreg);
                emitter.emit("imul", result_vreg, right_vreg);
            }
            break;
        case Token::SLASH:
            if (is_fp) {
                emitter.emit(is_double ? "vdivsd" : "vdivss", result_vreg,
                             left_vreg + ", " + right_vreg);
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
            if (node->op_type == Token::EQUAL_EQUAL) {
                set_instr = "sete";
            } else if (node->op_type == Token::BANG_EQUAL) {
                set_instr = "setne";
            } else if (node->op_type == Token::LESS) {
                set_instr = "setl";
            } else if (node->op_type == Token::GREATER) {
                set_instr = "setg";
            }

            emitter.emit(set_instr, "al");
            emitter.emit("movzx", result_vreg, "al");
            break;
    }

    last_expr_vreg = result_vreg;
}

void CodeGenerator::visit(GateAppOperationExpressionNode* node) {
    visit(node->qubit.get());
    std::string qubit_offset_vreg = last_expr_vreg;

    if (node->gate->node_type == ASTNode::NodeType::VARIABLE_REFERENCE) {
        auto* gate_ref = static_cast<VariableReferenceNode*>(node->gate.get());
        std::string name = gate_ref->name;

        static std::unordered_map<std::string, std::string> optimized_gates = {
            {"h", "q_h"}, {"x", "q_x"}, {"z", "q_z"}, {"s", "q_s"}, {"t", "q_t"}
        };

        if (optimized_gates.count(name)) {
            emitter.emit("mov", "rdi", qubit_offset_vreg);
            emitter.call_external(optimized_gates[name]);
            return;
        }
    }

    visit(node->gate.get());
    std::string matrix_addr_vreg = last_expr_vreg;

    emitter.emit("mov", "rdi", matrix_addr_vreg);
    emitter.emit("mov", "rsi", qubit_offset_vreg);
    emitter.emit("call", "q_apply_matrix");
}

void CodeGenerator::visit(PrintStatementNode* node) {
    for (size_t i = 0; i < node->expressions.size(); ++i) {
        visit(node->expressions[i].get());

        auto expr_type = node->expressions[i]->resolved_type;
        int size = getTypeSize(expr_type.get());

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
    for (const auto& stmt : node->true_block) {
        visit(stmt.get());
    }
    emitter.emit("jmp", end_label);

    // False block
    emitter.label(false_label);
    for (const auto& stmt : node->false_block) {
        visit(stmt.get());
    }

    emitter.label(end_label);
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
    int id = while_counter++;
    std::string start_label = "_while_start_" + std::to_string(id);
    std::string end_label = "_while_end_" + std::to_string(id);

    emitter.label(start_label);

    visit(node->condition.get());
    emitter.emit("cmp", last_expr_vreg, "0");
    emitter.emit("je", end_label);

    for (const auto& stmt : node->body) {
        visit(stmt.get());
    }

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

void CodeGenerator::visit(QubitDefinitionNode* node) {
    Symbol* sym = symbolTable.lookup(node->qubit_name);
    int q_offset = node->qubit_index * 32;

    std::string addr_vreg = new_vreg();
    emitter.emit("lea", addr_vreg, "[r15 + " + std::to_string(q_offset) + "]");

    std::string imm_vreg = new_vreg();
    emitter.emit("mov", imm_vreg, std::to_string(q_offset));
    emitter.emit_adv(8, sym->dataType.get(), "rbp", sym->offset, imm_vreg);

    if (node->has_custom_amplitudes) {
        visit(node->alpha.get());
        emitter.emit("vmovupd", "[" + addr_vreg + " + 0]", last_expr_vreg);
        visit(node->beta.get());
        emitter.emit("vmovupd", "[" + addr_vreg + " + 16]", last_expr_vreg);
    } else {
        emitter.emit("mov", "rdi", addr_vreg);
        emitter.call_external("q_init");
    }
}

void CodeGenerator::visit(FunctionCallNode* node) {
    // Intrinsics
    if (node->function_name == "exit") {
        if (!node->arguments.empty()) {
            visit(node->arguments[0].get());
            emitter.emit("mov", "rdi", last_expr_vreg);
        } else {
            emitter.emit("xor", "rdi", "rdi");
        }
        emitter.call_external("ny_exit");
        return;
    } if (node->function_name.rfind("__builtin_", 0) == 0) {
        if (node->arguments.empty()) {
            throw std::runtime_error("Codegen Error: Function " + node->function_name + " expectes at least 1 argument.");
        }

        visit(node->arguments[0].get());
        std::string_view op = std::string_view(node->function_name).substr(10);

        if (op == "sqrt") emitter.emit("sqrtsd", last_expr_vreg, last_expr_vreg);
        else if (op == "round") emitter.emit("roundsd", last_expr_vreg, last_expr_vreg + ", 0");
        else if (op == "abs") emitter.emit("vandpd", last_expr_vreg, last_expr_vreg + ", [rel _abs_mask]");

        return;
    }

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

    emitter.emit("mov", member_addr_vreg, base_addr_vreg);
    if ((member_symbol != nullptr) && member_symbol->offset != 0) {
        emitter.emit("add", member_addr_vreg, std::to_string(member_symbol->offset));
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
    if (Utils::isTypeKeyword(node->op_type)) {
        visit(node->operand.get()); 
        std::string dest_vreg = new_vreg();
        emit_cast(node->operand->resolved_type.get(), node->resolved_type.get(), last_expr_vreg, dest_vreg);
        last_expr_vreg = dest_vreg;
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

    if (node->op_type == Token::STAR) {  // Dereference
        int size = getTypeSize(node->resolved_type.get());
        emitter.load_adv(size, node->resolved_type.get(), res_vreg, op_vreg, 0);
    } else if (node->op_type == Token::BANG) {  // Logical NOT
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
        auto* var_ref = dynamic_cast<VariableReferenceNode*>(node->array_expr.get());
        if (var_ref->resolved_symbol != nullptr) {
            emitter.emit_lea_stack(base_vreg, var_ref->resolved_symbol->offset);
        }
    } else {
        visit(node->array_expr.get());
        emitter.emit("mov", base_vreg, last_expr_vreg);
    }
    is_lvalue = was_lvalue;

    int element_size = 4;  // Default to int size
    if (node->array_expr->resolved_type &&
        node->array_expr->resolved_type->category == TypeNode::TypeCategory::ARRAY) {
        auto* arr_type = dynamic_cast<ArrayTypeNode*>(node->array_expr->resolved_type.get());
        element_size = getTypeSize(arr_type->base_type.get());
    }

    std::string offset_vreg = new_vreg();
    emitter.emit("mov", offset_vreg, index_vreg);
    emitter.emit("imul", offset_vreg, std::to_string(element_size));

    std::string final_addr_vreg = new_vreg();
    emitter.emit("mov", final_addr_vreg, base_vreg);
    emitter.emit("add", final_addr_vreg, offset_vreg);

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

void CodeGenerator::visit(ComplexLiteralExpressionNode* node) {
    // Create a unique key for the constant pool
    std::stringstream ss;
    ss << std::fixed << std::setprecision(10) << node->real << "_" << node->imaginary;
    std::string val_key = ss.str();

    if (constants_map.find(val_key) == constants_map.end()) {
        std::string label = "_complex_lit_" + std::to_string(string_label_counter++);
        constants_map[val_key] = label;

        // Complex is two 64-bit doubles side-by-side in memory
        std::string nasm_val = std::to_string(node->real) + ", " + std::to_string(node->imaginary);
        constants.push_back({label, "dq", nasm_val});
    }

    std::string vreg = new_vreg();
    emitter.emit("vmovupd", vreg, "[rel " + constants_map[val_key] + "]");

    last_expr_vreg = vreg;
}

void CodeGenerator::visit(AsmStatementNode* node) {
    for (const auto& line : node->lines) {
        emitter.emit(line);
    }
}

auto CodeGenerator::getTypeSize(const TypeNode* type) -> int {
    if (type == nullptr) {
        std::cerr << "Type is null" << std::endl;
        throw std::runtime_error("Code Generation Error: Attempted to get size of a null type.");
    }

    switch (type->category) {
        case TypeNode::TypeCategory::PRIMITIVE: {
            const auto* prim_type = dynamic_cast<const PrimitiveTypeNode*>(type);
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
                case Token::KEYWORD_COMPLEX: return 16;
                case Token::KEYWORD_MATRIX:  return 64;
                case Token::KEYWORD_QUBIT:   return 8;
                default:
                    throw std::runtime_error(
                        "Code Generation Error: Unknown primitive type (" +
                        std::to_string(static_cast<int>(prim_type->primitive_type)) +
                        ") for size calculation. (Type category: " +
                        std::to_string(static_cast<int>(type->category)) + ")");
            }
        }
        case TypeNode::TypeCategory::POINTER:
            return 8;
        case TypeNode::TypeCategory::ARRAY: {
            const auto* array_type = dynamic_cast<const ArrayTypeNode*>(type);
            int element_size = getTypeSize(array_type->base_type.get());
            if (array_type->size > 0) {
                return element_size * array_type->size;
            }
            return 0;
        }
        case TypeNode::TypeCategory::STRUCT: {
            const auto* struct_type = dynamic_cast<const StructTypeNode*>(type);
            Symbol* struct_def_symbol = symbolTable.lookup(struct_type->struct_name);
            if ((struct_def_symbol == nullptr) || !struct_def_symbol->structDef) {
                const auto& structs = symbolTable.getStructDefinitions();
                if (structs.count(struct_type->struct_name) != 0U) {
                    auto* struct_ptr = structs.at(struct_type->struct_name);
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
                "Code Generation Error: Unknown type category for size "
                "calculation.");
    }
}

auto CodeGenerator::isFloatingPoint(const TypeNode* type) -> bool {
    if (type == nullptr) {
        return false;
    }
    const auto* prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}

auto CodeGenerator::getRegisterName(const std::string& reg64, int size) -> std::string {
    if (size == 8) {
        return reg64;
    }

    static const std::unordered_map<std::string, std::string> map32 = {
        {"rdi", "edi"}, {"rsi", "esi"}, {"rdx", "edx"},
        {"rcx", "ecx"}, {"r8", "r8d"},  {"r9", "r9d"}};

    return map32.at(reg64);
}

void CodeGenerator::emit_cast(const TypeNode* from, const TypeNode* to, const std::string& src_vreg, const std::string& dest_vreg) {
    if (!from || !to) return;

    bool src_fp = isFloatingPoint(from);
    bool dest_fp = isFloatingPoint(to);
    int src_size = getTypeSize(from);
    int dest_size = getTypeSize(to);
    auto* prim_from = dynamic_cast<const PrimitiveTypeNode*>(from);

    // Qubit -> Int (measuring qubits)
    if (prim_from && prim_from->primitive_type == Token::KEYWORD_QUBIT && !dest_fp) {
        emitter.emit("mov", "rdi", src_vreg);
        emitter.call_external("q_measure");
        emitter.emit("mov", dest_vreg, "rax");
        return;
    }

    // Float -> Int (e.g., (int)float_var)
    if (src_fp && !dest_fp) {
        emitter.emit(src_size == 8 ? "vcvttsd2si" : "vcvttss2si", dest_vreg, src_vreg);
    } 
    // Int -> Float (e.g., (float)int_var)
    else if (!src_fp && dest_fp) {
        std::string instr = (dest_size == 8) ? "vcvtsi2sd" : "vcvtsi2ss";
        emitter.emit_raw(instr, {dest_vreg, dest_vreg, src_vreg});
    } 
    // Float -> Float (e.g., (double)float_var)
    else if (src_fp && dest_fp) {
        if (src_size == 4 && dest_size == 8) 
            emitter.emit_raw("vcvtss2sd", {dest_vreg, dest_vreg, src_vreg});
        else if (src_size == 8 && dest_size == 4)
            emitter.emit_raw("vcvtsd2ss", {dest_vreg, dest_vreg, src_vreg});
        else
            emitter.emit("vmovss", dest_vreg, src_vreg);
    } 
    // Int -> Int (Identity / Widening)
    else {
        emitter.emit("mov", dest_vreg, src_vreg);
    }
}
