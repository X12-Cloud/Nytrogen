#include "code_generator.hpp"
#include "instruction_set.hpp"

#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <unordered_set>

CodeGenerator::CodeGenerator(std::unique_ptr<ProgramNode>& ast, SymbolTable& symTable)
    : program_ast(ast),
      symbolTable(symTable),
      string_label_counter(0),
      emitter(out)
{}

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
        if (emitted_data_labels.find(c.label) == emitted_data_labels.end()) {
            emitter.emit_data_entry(c.label, c.type, c.value);
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
        emitter.extern_sym(node->mangled_name);
        return;  // No further code generation for extern functions
    }

    emitter.label_local(node->mangled_name);
    emitter.emit("push", "rbp");
    current_stack_depth += 8;
    emitter.emit("mov", "rbp", "rsp");

    emitter.emit("and", "rsp", "-16");
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
        emitter.emit("sub", "rsp", std::to_string(aligned_space));
        current_stack_depth += aligned_space;
    }

    // Push register arguments onto the stack
    const std::vector<std::string> arg_registers = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    int register_args_size = 0;
    for (int i = 0; i < node->parameters.size() && i < arg_registers.size(); ++i) {
        int offset = (i + 1) * -8;
        emitter.mov_indirect("rbp", offset, arg_registers[i]);
    }

    emitter.write_raw(body_buffer.str());
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

            std::string nasm_type = (size == 4)                 ? "dd"
                                    : ( is_string || size == 8) ? "dq"
                                    : (size == 1)               ? "db"
                                                                : "dw";

            constants.push_back({final_name, nasm_type, init_val});

            if (has_non_const_init) {
                visit(decl.initial_value.get());
                if (is_float || is_double) {
                    std::string instr = is_float ? "vmovss" : "vmovsd";
                    emitter.emit_mem_rel(instr, final_name, "xmm0");
                } else {
                    emitter.emit_mem_rel("mov", final_name, "rax");
                }
            }
        } else {
            if (decl.initial_value) {
                visit(decl.initial_value.get());
                emitter.emit_adv(size, node->type.get(), "rbp", symbol->offset,
                         (is_float || is_double) ? "xmm0" : "rax");
            }
        }
    }
}

void CodeGenerator::visit(VariableAssignmentNode* node) {
    auto type = node->left->resolved_type;
    bool is_fp = isFloatingPoint(type.get());
    auto literal = dynamic_cast<LiteralExpressionNode*>(node->right.get());

    visit(node->right.get());
    auto* var_ref = dynamic_cast<VariableReferenceNode*>(node->left.get());
    if ((var_ref != nullptr) && !var_ref->resolved_symbol->mangled_name.empty()) {
        std::string label = var_ref->resolved_symbol->mangled_name;
        if (is_fp) {
            std::string instr = (getTypeSize(type.get()) == 4) ? "vmovss" : "vmovsd";
            emitter.emit_mem_rel(instr, label, "xmm0");
        } else {
            int size = getTypeSize(node->left->resolved_type.get());
            if (size == 4) {
                emitter.emit("mov", "dword [rel " + label + "]", "eax");
            } else if (size == 1) {
                emitter.emit("mov", "byte [rel " + label + "]", "al");
            } else {
                emitter.emit("mov", "[rel " + label + "]", "rax");
            }
        }
    } else {
        int size = getTypeSize(node->left->resolved_type.get());
        if (is_fp) {
            is_lvalue = true;
            visit(node->left.get());
            is_lvalue = false;
            emitter.emit_adv(size, type.get(), "rax", 0, "xmm0");
        } else {
            emitter.emit("push", "rax");
            current_stack_depth += 8;
            is_lvalue = true;
            visit(node->left.get());
            is_lvalue = false;
            emitter.emit("pop", "rbx");
            current_stack_depth -= 8;
            emitter.emit_adv(size, type.get(), "rax", 0, "rbx");
        }
    }
}

void CodeGenerator::visit(VariableReferenceNode* node) {
    Symbol* symbol = node->resolved_symbol;
    int offset = symbol->offset;

    if (node->resolved_symbol == nullptr) {
        throw std::runtime_error("CodeGen Error: Symbol not resolved for " + node->name);
    }
    if (!node->resolved_symbol->dataType) {
        throw std::runtime_error("CodeGen Error: Variable '" + node->name +
                                 "' has NO TYPE in symbol table!");
    }

    if (symbol == nullptr) {
        throw std::runtime_error("CodeGen Error: Reference to '" + node->name + "' not resolved.");
    }

    if (symbol->type == Symbol::SymbolType::CONSTANT) {
        visit(symbol->value.get());
        return;
    }

    auto prim = dynamic_cast<PrimitiveTypeNode*>(node->resolved_type.get());
    bool is_double = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT);
    bool is_global = !symbol->mangled_name.empty() && symbol->mangled_name != symbol->name;
    int size = getTypeSize(node->resolved_type.get());

    if (is_global) {
        std::string asm_label = symbol->mangled_name;
        if (is_lvalue) {
            emitter.emit("lea", "rax", "[rel " + asm_label + "]");
        } else {
            if (is_float || is_double) {
                std::string instr = is_float ? "vmovss" : "vmovsd";
                emitter.emit(size == 4 ? "vmovss" : "vmovsd", "xmm0", "[rel " + asm_label + "]");
            } else {
                std::string instr = (size == 1) ? "movsx rax, byte" : (size == 4) ? "movsx rax, dword" : "mov rax,";
                emitter.emit(instr, "[rel " + asm_label + "]");
            }
        }
        return;
    }
    offset = symbol->offset;
    if (is_lvalue) {
        emitter.emit("lea", "rax", "[rbp + " + std::to_string(offset) + "]");
    } else {
        emitter.load_adv(size, node->resolved_type.get(), (is_float || is_double) ? "xmm0" : "rax", "rbp", offset);
    }
}

void CodeGenerator::visit(BinaryOperationExpressionNode* node) {
    bool is_float = false;
    bool is_double = false;

    if (node->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE) {
        auto prim = std::static_pointer_cast<PrimitiveTypeNode>(node->resolved_type);
        is_float = (prim->primitive_type == Token::KEYWORD_FLOAT ||
                    prim->primitive_type == Token::FLOAT_LITERAL);
        is_double = (prim->primitive_type == Token::KEYWORD_DOUBLE ||
                     prim->primitive_type == Token::DOUBLE_LITERAL);
    }
    if (debug_mode) {
        if (node->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE) {
            auto prim = std::static_pointer_cast<PrimitiveTypeNode>(node->resolved_type);
            std::cout << "Debug: Primitive Type ID found: " << prim->primitive_type << std::endl;
            std::cout << "Debug: Expected FLOAT_LITERAL: " << Token::FLOAT_LITERAL << std::endl;
            std::cout << "Debug: Expected KEYWORD_FLOAT: " << Token::KEYWORD_FLOAT << std::endl;
        }
    }

    // Left
    visit(node->left.get());
    if (is_float || is_double) {
        emitter.emit("sub", "rsp", "8");
        current_stack_depth += 8;
        if (is_double) {
            emitter.emit("vmovsd", "qword [rsp]", "xmm0");
        } else {
            emitter.emit("vmovss", "dword [rsp]", "xmm0");
        }
    } else {
        emitter.emit("push", "rax");
        current_stack_depth += 8;
    }

    // Right
    visit(node->right.get());

    if (is_float || is_double) {
        if (is_double) {
            emitter.emit("vmovsd", "xmm1", "qword [rsp]");
        } else {
            emitter.emit("vmovss", "xmm1", "dword [rsp]");
        }
        emitter.emit("add", "rsp", "8");  // Left is in xmm1, Right is in xmm0
        current_stack_depth -= 8;
    } else {
        emitter.emit("pop", "rbx");
        current_stack_depth -= 8;
        emitter.emit("add", "rcx", "rbx");  // Left is in rbx, Right is in rax
    }

    char type = 'd';
    bool is_string = false;
    if (is_float) {
        type = 'f';
    } else if (is_double) {
        type = 'l';
    }
    if (node->left->resolved_type && 
        node->left->resolved_type->category == TypeNode::TypeCategory::PRIMITIVE) {
        auto prim = static_cast<PrimitiveTypeNode*>(node->left->resolved_type.get());
        is_string = (prim->primitive_type == Token::KEYWORD_STRING);
    }

    switch (node->op_type) {
        case Token::PLUS:  emitter.emit_binary_op("add", type); break;
        case Token::MINUS: emitter.emit_binary_op("sub", type); break;
        case Token::STAR:  emitter.emit_binary_op("imul", type); break;
        case Token::SLASH: emitter.emit_binary_op("idiv", type); break;
        case Token::EQUAL_EQUAL:   emitter.emit_cmp("sete", is_string); break;
        case Token::BANG_EQUAL:    emitter.emit_cmp("setne", is_string); break;
        case Token::LESS:          emitter.emit_cmp("setl", false); break;
        case Token::GREATER:       emitter.emit_cmp("setg", false); break;
        case Token::LESS_EQUAL:    emitter.emit_cmp("setle", false); break;
        case Token::GREATER_EQUAL: emitter.emit_cmp("setge", false); break;
        default: throw std::runtime_error("Unknown operator");
    }
}

void CodeGenerator::visit(PrintStatementNode* node) {
    for (const auto& expr : node->expressions) {
        visit(expr.get());

        auto expr_type = expr->resolved_type;
        int size = getTypeSize(expr_type.get());
        if (!expr_type) {
            // Fallback if semantic analysis missed a type
            emitter.emit_print_int("rax");
            continue;
        }
        emitter.emit_print(size, expr_type);
    }
}

void CodeGenerator::visit(ReturnStatementNode* node) {
    if (node->expression) {
        auto* var_ref = dynamic_cast<VariableReferenceNode*>(node->expression.get());

        if (var_ref && var_ref->resolved_symbol && var_ref->resolved_symbol->offset < 0) {
            int size = getTypeSize(var_ref->resolved_type.get());
            emitter.load_adv(size, var_ref->resolved_type.get(), "rax", "rbp", var_ref->resolved_symbol->offset);
        } else {
            visit(node->expression.get());
        }
    }
    emitter.emit("jmp", current_function_name + "_epilogue");
}

void CodeGenerator::visit(IfStatementNode* node) {
    static int if_counter = 0;
    int id = if_counter++;

    std::string false_label = "_if_false_" + std::to_string(id);
    std::string end_label = "_if_end_" + std::to_string(id);

    visit(node->condition.get());
    emitter.emit("cmp", "rax", "0");
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
    emitter.emit("cmp", "rax", "0");
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
        emitter.emit("cmp", "rax", "0");
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
    const std::vector<std::string> arg_regs_64 = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};
    const std::vector<std::string> arg_regs_32 = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
    int arg_count = node->arguments.size();

    if (node->resolved_symbol == nullptr) {
        throw std::runtime_error("CodeGen Error: Function " + node->function_name + " not found.");
    }

    for (int i = arg_count - 1; i >= (int)arg_regs_64.size(); --i) {
        visit(node->arguments[i].get());
        emitter.emit("push", "rax");
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
            emitter.emit("mov", arg_regs_64[i], "rax");
        } else {
            emitter.emit("mov", arg_regs_32[i], "eax");
        }
    }

    std::string target_label = node->resolved_symbol->mangled_name;

    emitter.emit("call", target_label);

    if (arg_count > arg_regs_64.size()) {
        int cleanup = (arg_count - arg_regs_64.size()) * 8;
        emitter.emit("add", "rsp", std::to_string(cleanup));
        current_stack_depth -= cleanup;
    }
}

void CodeGenerator::visit(MemberAccessNode* node) {
    bool old_lvalue = is_lvalue;
    is_lvalue = true;
    visit(node->struct_expr.get());
    is_lvalue = old_lvalue;

    Symbol* member_symbol = node->resolved_symbol;
    if ((member_symbol != nullptr) && member_symbol->offset != 0) {
        emitter.emit("add", "rax", std::to_string(member_symbol->offset));
    }

    if (!node->resolved_type) {
        throw std::runtime_error("CodeGen Error: Member access '" + node->member_name +
                                 "' has no resolved type.");
    }

    if (!is_lvalue) {
        int size = getTypeSize(node->resolved_type.get());
        emitter.load_from_address(size, "rax"); 
    }
}

void CodeGenerator::visit(UnaryOpExpressionNode* node) {
    if (node->op_type == Token::KEYWORD_INT || node->op_type == Token::KEYWORD_CHAR) {
        visit(node->operand.get());
        if (isFloatingPoint(node->operand->resolved_type.get())) {
            int size = getTypeSize(node->operand->resolved_type.get());
            emitter.emit(size == 8 ? "cvtsd2si" : "cvtss2si", "rax", "xmm0");
        }
        return;
    }

    if (node->op_type == Token::ADDRESSOF) {
        const auto* ref_node = static_cast<const VariableReferenceNode*>(node->operand.get());
        if (!ref_node->resolved_symbol) {
             throw std::runtime_error("CodeGen Error: Symbol not resolved for " + ref_node->name);
        }
        emitter.emit_lea_stack(ref_node->resolved_symbol->offset);
        return;
    }

    visit(node->operand.get());
    if (node->op_type == Token::STAR) {
        emitter.emit_dereference();
    } else if (node->op_type == Token::BANG) {
        emitter.emit("test", "rax", "rax");
        emitter.emit("setz", "al");
        emitter.emit("movzx", "rax", "al");
    }
}

void CodeGenerator::visit(ArrayAccessNode* node) {
    bool was_lvalue = is_lvalue;
    is_lvalue = false;
    visit(node->index_expr.get());
    emitter.emit("mov", "rbx", "rax");
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
        if (var_ref->resolved_symbol) {
            emitter.emit_lea_stack(var_ref->resolved_symbol->offset);
        } else {
            throw std::runtime_error("CodeGen Error: Symbol not found.");
        }
    } else {
        visit(node->array_expr.get());
    }

    emitter.emit("imul", "rbx", std::to_string(element_size));
    emitter.emit("add", "rax", "rbx");

    if (!was_lvalue) {
        // Reuse your handy helper!
        emitter.load_from_address(element_size, "rax");
    }
}

void CodeGenerator::visit(StructDefinitionNode* node) {
    // No code generation needed for struct definitions
}

void CodeGenerator::visit(IntegerLiteralExpressionNode* node) {
    emitter.emit("mov", "rax", std::to_string(node->value));

    if (!node->resolved_type) {
        node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_INT);
    }
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

    emitter.emit_load_constant("vmovss", constants_map[val_str]);
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

    emitter.emit_load_constant("vmovsd", constants_map[val_str]);
}

void CodeGenerator::visit(StringLiteralExpressionNode* node) {
    std::string formatted_val = "\"" + unescapeString(node->value) + "\", 0";
    if (constants_map.find(formatted_val) == constants_map.end()) {
        std::string label = "_str_" + std::to_string(string_label_counter++);
        constants_map[formatted_val] = label;
        constants.push_back({label, "db", formatted_val});
    }

    emitter.emit("lea", "rax", "[rel " + constants_map[formatted_val] + "]");
    node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_STRING);
}

void CodeGenerator::visit(BooleanLiteralExpressionNode* node) {
    emitter.emit("mov", "rax", std::to_string(node->value ? 1 : 0));
}

void CodeGenerator::visit(CharacterLiteralExpressionNode* node) {
    emitter.emit("mov", "rax", std::to_string(static_cast<int>(node->value)));
    if (!node->resolved_type) {
        node->resolved_type = std::make_shared<PrimitiveTypeNode>(Token::KEYWORD_BOOL);
    }
}

void CodeGenerator::visit(AsmStatementNode* node) {
    for (const auto& line : node->lines) {
        //out << "    " << line << "\n";
        emitter.emit(line);
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

bool CodeGenerator::isFloatingPoint(const TypeNode* type) {
    if (type == nullptr) {
        return false;
    }
    auto prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}
