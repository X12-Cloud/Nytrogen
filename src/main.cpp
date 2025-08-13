#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <unordered_map>
#include <stdexcept>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"
#include "semantic_analyzer.hpp"

std::unordered_map<std::string, std::string> string_literals;
int string_label_counter = 0;

std::string generateCode(const ASTNode* node, SemanticAnalyzer& semanticAnalyzer, bool as_lvalue = false);
std::string generateFunctionCode(const FunctionDefinitionNode* func_node, SemanticAnalyzer& semanticAnalyzer);
std::string escapeString(const std::string& raw);

std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::cout << "Nytrogen Compiler (Arch Linux)\n";

    if (argc < 2) {
        std::cerr << "Error: No source file provided. Usage: ./Nytro <source_file>\n";
        return 2;
    }

    std::string input_filepath = argv[1];
    std::string output_asm_filename = "out/out.asm";

    std::string ext = input_filepath.substr(input_filepath.find_last_of(".") + 1);
    if (ext != "ny" && ext != "nyt") {
        std::cerr << "Error: Input file must have .ny or .nyt extension (found: ." << ext << ")\n";
        return 3;
    }

    std::string sourceCode = readFileContent(input_filepath);
    if (sourceCode.empty()) return 2;

    std::cout << "\n--- Processing Source File: " << input_filepath << " ---\n\n";

    std::vector<Token> tokens = tokenize(sourceCode);
    Parser parser(std::move(tokens));
    std::unique_ptr<ProgramNode> ast_root = parser.parse();

    if (!ast_root) {
        std::cerr << "AST generation failed during parsing. Exiting.\n";
        return 1;
    }

    // Perform semantic analysis
    SemanticAnalyzer semanticAnalyzer(ast_root, parser.getSymbolTable());
    semanticAnalyzer.analyze();

    // Debug: Print offsets of 'a' and 'b' in 'add' function
    Symbol* a_sym = semanticAnalyzer.getSymbolTable().lookup("a");
    if (a_sym) {
        std::cout << "Debug: Offset of 'a': " << a_sym->offset << std::endl;
    }
    Symbol* b_sym = semanticAnalyzer.getSymbolTable().lookup("b");
    if (b_sym) {
        std::cout << "Debug: Offset of 'b': " << b_sym->offset << std::endl;
    }

    std::ofstream asm_file(output_asm_filename);
    if (!asm_file.is_open()) {
        std::cerr << "Error: Could not create output assembly file: " << output_asm_filename << "\n";
        return 2;
    }

    std::string text_section;
    for (const auto& func : ast_root->functions) {
        text_section += generateFunctionCode(func.get(), semanticAnalyzer);
    }

    asm_file << "section .data\n";
    asm_file << "  _print_int_format db \"%d\", 10, 0\n";
    asm_file << "  _print_str_format db \"%s\", 10, 0\n";
    asm_file << "  _print_char_format db \"%c\", 10, 0\n";
    for (const auto& pair : string_literals) {
        asm_file << "  " << pair.second << " db " << escapeString(pair.first) << ", 0\n";
    }

    asm_file << "\nsection .text\n";
    asm_file << "global _start\n";
    asm_file << "extern printf\n";
    asm_file << text_section;
    asm_file << "_start:\n";
    asm_file << "  call main\n";
    asm_file << "  mov rdi, rax\n";
    asm_file << "  mov rax, 60\n";
    asm_file << "  syscall\n";

    asm_file.close();
    std::cout << "Successfully generated assembly to '" << output_asm_filename << "'\n";
    return 0;
}

std::string generateCode(const ASTNode* node, SemanticAnalyzer& semanticAnalyzer, bool as_lvalue) {
    if (!node) throw std::runtime_error("Code generation error: null AST node");

    std::string assembly;

    switch (node->node_type) {
        case ASTNode::NodeType::STRING_LITERAL_EXPRESSION: {
            const auto* str_node = static_cast<const StringLiteralExpressionNode*>(node);
            const std::string& value = str_node->value;

            std::string label;
            if (string_literals.count(value)) {
                label = string_literals[value];
            } else {
                label = "_str_" + std::to_string(string_label_counter++);
                string_literals[value] = label;
            }

            assembly += "  lea rax, [rel " + label + "]\n";
            break;
        }

        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION: {
            const auto* int_node = static_cast<const IntegerLiteralExpressionNode*>(node);
            assembly += "  mov rax, " + std::to_string(int_node->value) + "\n";
            break;
        }

        case ASTNode::NodeType::BOOLEAN_LITERAL_EXPRESSION: {
            const auto* bool_node = static_cast<const BooleanLiteralExpressionNode*>(node);
            assembly += "  mov rax, " + std::to_string(bool_node->value) + "\n";
            break;
        }

        case ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION: {
            const auto* char_node = static_cast<const CharacterLiteralExpressionNode*>(node);
            assembly += "  mov rax, " + std::to_string(static_cast<int>(char_node->value)) + "\n";
            break;
        }

        case ASTNode::NodeType::VARIABLE_REFERENCE: {
            const auto* ref_node = static_cast<const VariableReferenceNode*>(node);
            int offset = ref_node->resolved_offset;
            if (as_lvalue) {
                assembly += "  lea rax, [rbp + " + std::to_string(offset) + "]\n";
            } else {
                assembly += "  mov rax, qword [rbp + " + std::to_string(offset) + "]\n";
            }
            break;
        }

        case ASTNode::NodeType::VARIABLE_ASSIGNMENT: {
            const auto* assign_node = static_cast<const VariableAssignmentNode*>(node);
            assembly += generateCode(assign_node->right.get(), semanticAnalyzer, false);
            assembly += "  push rax\n";
            assembly += generateCode(assign_node->left.get(), semanticAnalyzer, true);
            assembly += "  pop rbx\n";
            assembly += "  mov [rax], rbx\n";
            break;
        }

        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            const auto* bin = static_cast<const BinaryOperationExpressionNode*>(node);
            assembly += generateCode(bin->left.get(), semanticAnalyzer);
            assembly += "  push rax\n";
            assembly += generateCode(bin->right.get(), semanticAnalyzer);
            assembly += "  pop rbx\n";

            switch (bin->op_type) {
                case Token::PLUS: assembly += "  add rbx, rax\n  mov rax, rbx\n"; break;
                case Token::MINUS: assembly += "  sub rbx, rax\n  mov rax, rbx\n"; break;
                case Token::STAR: assembly += "  imul rax, rbx\n"; break;
                case Token::SLASH:
                    assembly += "  mov rcx, rax\n";
                    assembly += "  mov rax, rbx\n";
                    assembly += "  mov rbx, rcx\n";
                    assembly += "  xor rdx, rdx\n";
                    assembly += "  idiv rbx\n";
                    break;
                case Token::EQUAL_EQUAL:
                    assembly += "  cmp rbx, rax\n";
                    assembly += "  sete al\n  movzx rax, al\n";
                    break;
                case Token::BANG_EQUAL:
                    assembly += "  cmp rbx, rax\n";
                    assembly += "  setne al\n  movzx rax, al\n";
                    break;
                case Token::LESS:
                    assembly += "  cmp rbx, rax\n";
                    assembly += "  setl al\n  movzx rax, al\n";
                    break;
                case Token::GREATER:
                    assembly += "  cmp rbx, rax\n";
                    assembly += "  setg al\n  movzx rax, al\n";
                    break;
                case Token::LESS_EQUAL:
                    assembly += "  cmp rbx, rax\n";
                    assembly += "  setle al\n  movzx rax, al\n";
                    break;
                case Token::GREATER_EQUAL:
                    assembly += "  cmp rbx, rax\n";
                    assembly += "  setge al\n  movzx rax, al\n";
                    break;
                default:
                    throw std::runtime_error("Unknown binary operator.");
            }
            break;
        }

        case ASTNode::NodeType::RETURN_STATEMENT: {
            const auto* ret = static_cast<const ReturnStatementNode*>(node);
            assembly += generateCode(ret->expression.get(), semanticAnalyzer);
            break;
        }

        case ASTNode::NodeType::IF_STATEMENT: {
            const auto* if_node = static_cast<const IfStatementNode*>(node);
            static int if_counter = 0;
            int label_id = if_counter++;

            std::string true_label = ".if_true_" + std::to_string(label_id);
            std::string false_label = ".if_false_" + std::to_string(label_id);
            std::string end_label = ".if_end_" + std::to_string(label_id);

            assembly += generateCode(if_node->condition.get(), semanticAnalyzer);
            assembly += "  cmp rax, 0\n";
            assembly += "  je " + false_label + "\n";

            assembly += true_label + ":\n";
            for (const auto& stmt : if_node->true_block) {
                assembly += generateCode(stmt.get(), semanticAnalyzer);
            }
            assembly += "  jmp " + end_label + "\n";

            assembly += false_label + ":\n";
            for (const auto& stmt : if_node->false_block) {
                assembly += generateCode(stmt.get(), semanticAnalyzer);
            }

            assembly += end_label + ":\n";
            break;
        }

        case ASTNode::NodeType::WHILE_STATEMENT: {
            const auto* while_node = static_cast<const WhileStatementNode*>(node);
            static int while_counter = 0;
            int label_id = while_counter++;

            std::string start_label = ".while_start_" + std::to_string(label_id);
            std::string end_label = ".while_end_" + std::to_string(label_id);

            assembly += start_label + ":\n";
            assembly += generateCode(while_node->condition.get(), semanticAnalyzer);
            assembly += "  cmp rax, 0\n";
            assembly += "  je " + end_label + "\n";

            for (const auto& stmt : while_node->body) {
                assembly += generateCode(stmt.get(), semanticAnalyzer);
            }

            assembly += "  jmp " + start_label + "\n";
            assembly += end_label + ":\n";
            break;
        }

        case ASTNode::NodeType::FOR_STATEMENT: {
            const auto* for_node = static_cast<const ForStatementNode*>(node);
            static int for_counter = 0;
            int label_id = for_counter++;

            std::string loop_start_label = ".for_loop_start_" + std::to_string(label_id);
            std::string loop_condition_label = ".for_loop_condition_" + std::to_string(label_id);
            std::string loop_end_label = ".for_loop_end_" + std::to_string(label_id);

            if (for_node->initializer) {
                assembly += generateCode(for_node->initializer.get(), semanticAnalyzer);
            }
            assembly += "  jmp " + loop_condition_label + "\n";

            assembly += loop_start_label + ":\n";
            for (const auto& stmt : for_node->body) {
                assembly += generateCode(stmt.get(), semanticAnalyzer);
            }
            if (for_node->increment) {
                assembly += generateCode(for_node->increment.get(), semanticAnalyzer);
            }

            assembly += loop_condition_label + ":\n";
            if (for_node->condition) {
                assembly += generateCode(for_node->condition.get(), semanticAnalyzer);
                assembly += "  cmp rax, 0\n";
                assembly += "  jne " + loop_start_label + "\n";
            } else { // No condition means infinite loop, jump directly to start
                assembly += "  jmp " + loop_start_label + "\n";
            }
            assembly += loop_end_label + ":\n";
            break;
        }

        case ASTNode::NodeType::VARIABLE_DECLARATION: {
            const auto* decl_node = static_cast<const VariableDeclarationNode*>(node);
            Symbol* var_symbol = decl_node->resolved_symbol;
            if (!var_symbol) {
                throw std::runtime_error("Code generation error: variable '" + decl_node->name + "' not found in symbol table during code generation.");
            }
            int var_size = var_symbol->size;

            // Align stack to 8 bytes
            if (var_size % 8 != 0) {
                var_size = (var_size / 8 + 1) * 8;
            }

            int offset = var_symbol->offset;

            assembly += "  sub rsp, " + std::to_string(var_size) + "\n"; // Adjust stack pointer

            if (decl_node->initial_value) {
                assembly += generateCode(decl_node->initial_value.get(), semanticAnalyzer);
                assembly += "  mov qword [rbp + " + std::to_string(offset) + "], rax\n";
            } else {
                // Initialize to 0 if no initial value is provided
                assembly += "  mov qword [rbp + " + std::to_string(offset) + "], 0\n";
            }
            break;
        }

        case ASTNode::NodeType::PRINT_STATEMENT: {
            const auto* print_node = static_cast<const PrintStatementNode*>(node);
            assembly += generateCode(print_node->expression.get(), semanticAnalyzer);

            if (print_node->expression->node_type == ASTNode::NodeType::STRING_LITERAL_EXPRESSION) {
                assembly += "  mov rsi, rax\n";
                assembly += "  lea rdi, [rel _print_str_format]\n";
            } else if (print_node->expression->node_type == ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION) {
                assembly += "  mov rsi, rax\n";
                assembly += "  lea rdi, [rel _print_char_format]\n";
            }
            else {
                assembly += "  mov rsi, rax\n";
                assembly += "  lea rdi, [rel _print_int_format]\n";
            }

            assembly += "  xor rax, rax\n";
            assembly += "  call printf\n";
            break;
        }

        case ASTNode::NodeType::FUNCTION_CALL: {
            const auto* call_node = static_cast<const FunctionCallNode*>(node);
            for (int i = call_node->arguments.size() - 1; i >= 0; --i) {
                assembly += generateCode(call_node->arguments[i].get(), semanticAnalyzer);
                assembly += "  push rax\n";
            }
            assembly += "  call " + call_node->function_name + "\n";
            if (!call_node->arguments.empty()) {
                assembly += "  add rsp, " + std::to_string(call_node->arguments.size() * 8) + "\n";
            }
            break;
        }

        case ASTNode::NodeType::UNARY_OP_EXPRESSION: {
            const auto* unary_node = static_cast<const UnaryOpExpressionNode*>(node);
            assembly += generateCode(unary_node->operand.get(), semanticAnalyzer);
            if (unary_node->op_type == Token::ADDRESSOF) {
                // The operand should be a variable reference, get its address
                const auto* ref_node = static_cast<const VariableReferenceNode*>(unary_node->operand.get());
                Symbol* var_symbol = ref_node->resolved_symbol;
                if (!var_symbol) {
                    throw std::runtime_error("Code generation error: variable '" + ref_node->name + "' used before declaration for address-of (resolved_symbol is null).");
                }
                int offset = var_symbol->offset;
                assembly += "  lea rax, [rbp + " + std::to_string(offset) + "]\n";
            } else if (unary_node->op_type == Token::STAR) {
                // The operand is a pointer, so rax holds the address. Dereference it.
                assembly += "  mov rax, [rax]\n";
            }
            break;
        }

        case ASTNode::NodeType::ARRAY_ACCESS_EXPRESSION: {
            const auto* access_node = static_cast<const ArrayAccessNode*>(node);
            assembly += generateCode(access_node->index_expr.get(), semanticAnalyzer);
            assembly += "  mov rbx, rax\n";
            const auto* ref_node = static_cast<const VariableReferenceNode*>(access_node->array_expr.get());
            Symbol* var_symbol = ref_node->resolved_symbol;
            if (!var_symbol) {
                throw std::runtime_error("Code generation error: array variable '" + ref_node->name + "' not found for access (resolved_symbol is null).");
            }
            int offset = var_symbol->offset;
            assembly += "  lea rcx, [rbp + " + std::to_string(offset) + "]\n";
            assembly += "  imul rbx, 8\n";
            assembly += "  add rcx, rbx\n";
            if (as_lvalue) {
                assembly += "  mov rax, rcx\n";
            } else {
                assembly += "  mov rax, [rcx]\n";
            }
            break;
        }

        case ASTNode::NodeType::MEMBER_ACCESS_EXPRESSION: {
            const auto* member_access_node = static_cast<const MemberAccessNode*>(node);
            assembly += generateCode(member_access_node->struct_expr.get(), semanticAnalyzer, true);

            const VariableReferenceNode* var_ref_node = static_cast<const VariableReferenceNode*>(member_access_node->struct_expr.get());
            Symbol* struct_symbol = var_ref_node->resolved_symbol;
            if (!struct_symbol || struct_symbol->dataType->category != TypeNode::TypeCategory::STRUCT) {
                throw std::runtime_error("Code generation error: Member access on non-struct variable '" + var_ref_node->name + "'.");
            }
            const StructTypeNode* struct_type = static_cast<const StructTypeNode*>(struct_symbol->dataType.get());
            Symbol* struct_def_symbol = semanticAnalyzer.getSymbolTable().lookup(struct_type->struct_name);
            if (!struct_def_symbol || !struct_def_symbol->structDef) {
                throw std::runtime_error("Code generation error: Struct definition for '" + struct_type->struct_name + "' not found.");
            }
            const auto& struct_def = struct_def_symbol->structDef;

            int member_offset = -1;
            for (const auto& member : struct_def->members) {
                if (member.name == member_access_node->member_name) {
                    member_offset = member.offset;
                    break;
                }
            }

            if (member_offset == -1) {
                throw std::runtime_error("Code generation error: Member '" + member_access_node->member_name + "' not found in struct '" + struct_type->struct_name + "'.");
            }

            assembly += "  add rax, " + std::to_string(member_offset) + "\n";
            if (!as_lvalue) {
                assembly += "  mov rax, [rax]\n";
            }
            break;
        }

        case ASTNode::NodeType::FUNCTION_DEFINITION:
        case ASTNode::NodeType::PROGRAM:
        case ASTNode::NodeType::STRUCT_DEFINITION:
            throw std::runtime_error("Unexpected node in generateCode.");
    }

    return assembly;
}

std::string generateFunctionCode(const FunctionDefinitionNode* func_node, SemanticAnalyzer& semanticAnalyzer) {
    std::string assembly;

    assembly += func_node->name + ":\n";
    assembly += "  push rbp\n";
    assembly += "  mov rbp, rsp\n";

    for (const auto& stmt : func_node->body_statements) {
        assembly += generateCode(stmt.get(), semanticAnalyzer);
    }

    if (func_node->name == "main") {
        assembly += ".main_epilogue:\n";
    }

    assembly += "  mov rsp, rbp\n";
    assembly += "  pop rbp\n";
    assembly += "  ret\n\n";

    return assembly;
}

std::string escapeString(const std::string& raw) {
    std::string out = "\"";
    for (char c : raw) {
        if (c == '\\') out += "\\";
        else if (c == '\"') out += "\"";
        else if (c == '\n') out += "\n";
        else out += c;
    }
    out += "\"";
    return out;
}
