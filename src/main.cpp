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

std::map<std::string, int> stack_offsets;
int current_stack_offset = 0;

std::unordered_map<std::string, std::string> string_literals;
int string_label_counter = 0;

std::string generateCode(const ASTNode* node);
std::string generateFunctionCode(const FunctionDefinitionNode* func_node);
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

    std::ofstream asm_file(output_asm_filename);
    if (!asm_file.is_open()) {
        std::cerr << "Error: Could not create output assembly file: " << output_asm_filename << "\n";
        return 2;
    }

    std::string text_section;
    for (const auto& func : ast_root->functions) {
        text_section += generateFunctionCode(func.get());
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

std::string generateCode(const ASTNode* node) {
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
            if (stack_offsets.find(ref_node->name) == stack_offsets.end()) {
                throw std::runtime_error("Code generation error: variable '" + ref_node->name + "' used before declaration.");
            }
            int offset = stack_offsets[ref_node->name];
            assembly += "  mov rax, qword [rbp + " + std::to_string(offset) + "]\n";
            break;
        }

        case ASTNode::NodeType::VARIABLE_ASSIGNMENT: {
            const auto* assign_node = static_cast<const VariableAssignmentNode*>(node);
            if (stack_offsets.find(assign_node->name) == stack_offsets.end()) {
                throw std::runtime_error("Code generation error: variable '" + assign_node->name + "' assigned before declaration.");
            }
            assembly += generateCode(assign_node->expression.get());
            int offset = stack_offsets[assign_node->name];
            assembly += "  mov qword [rbp + " + std::to_string(offset) + "], rax\n";
            break;
        }

        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            const auto* bin = static_cast<const BinaryOperationExpressionNode*>(node);
            assembly += generateCode(bin->left.get());
            assembly += "  push rax\n";
            assembly += generateCode(bin->right.get());
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
            assembly += generateCode(ret->expression.get());
            //assembly += "  jmp .main_epilogue\n";
            break;
        }

        case ASTNode::NodeType::IF_STATEMENT: {
            const auto* if_node = static_cast<const IfStatementNode*>(node);
            static int if_counter = 0;
            int label_id = if_counter++;

            std::string true_label = ".if_true_" + std::to_string(label_id);
            std::string false_label = ".if_false_" + std::to_string(label_id);
            std::string end_label = ".if_end_" + std::to_string(label_id);

            assembly += generateCode(if_node->condition.get());
            assembly += "  cmp rax, 0\n";
            assembly += "  je " + false_label + "\n";

            assembly += true_label + ":\n";
            for (const auto& stmt : if_node->true_block) {
                assembly += generateCode(stmt.get());
            }
            assembly += "  jmp " + end_label + "\n";

            assembly += false_label + ":\n";
            for (const auto& stmt : if_node->false_block) {
                assembly += generateCode(stmt.get());
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
            assembly += generateCode(while_node->condition.get());
            assembly += "  cmp rax, 0\n";
            assembly += "  je " + end_label + "\n";

            for (const auto& stmt : while_node->body) {
                assembly += generateCode(stmt.get());
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
                assembly += generateCode(for_node->initializer.get());
            }
            assembly += "  jmp " + loop_condition_label + "\n";

            assembly += loop_start_label + ":\n";
            for (const auto& stmt : for_node->body) {
                assembly += generateCode(stmt.get());
            }
            if (for_node->increment) {
                assembly += generateCode(for_node->increment.get());
            }

            assembly += loop_condition_label + ":\n";
            if (for_node->condition) {
                assembly += generateCode(for_node->condition.get());
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
            current_stack_offset -= 8; // Allocate space for the new variable
            stack_offsets[decl_node->name] = current_stack_offset;
            assembly += "  sub rsp, 8\n"; // Adjust stack pointer

            if (decl_node->initial_value) {
                assembly += generateCode(decl_node->initial_value.get());
                assembly += "  mov qword [rbp + " + std::to_string(current_stack_offset) + "], rax\n";
            } else {
                // Initialize to 0 if no initial value is provided
                assembly += "  mov qword [rbp + " + std::to_string(current_stack_offset) + "], 0\n";
            }
            break;
        }

        case ASTNode::NodeType::PRINT_STATEMENT: {
            const auto* print_node = static_cast<const PrintStatementNode*>(node);
            assembly += generateCode(print_node->expression.get());

            if (print_node->expression->node_type == ASTNode::NodeType::STRING_LITERAL_EXPRESSION) {
                assembly += "  mov rsi, rax\n";
                assembly += "  lea rdi, [rel _print_str_format]\n";
            } else if (print_node->expression->node_type == ASTNode::NodeType::CHARACTER_LITERAL_EXPRESSION) {
                assembly += "  mov rsi, rax\n";
                assembly += "  lea rdi, [rel _print_char_format]\n";
            } else {
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
                assembly += generateCode(call_node->arguments[i].get());
                assembly += "  push rax\n";
            }
            assembly += "  call " + call_node->function_name + "\n";
            if (!call_node->arguments.empty()) {
                assembly += "  add rsp, " + std::to_string(call_node->arguments.size() * 8) + "\n";
            }
            break;
        }

        case ASTNode::NodeType::FUNCTION_DEFINITION:
        case ASTNode::NodeType::PROGRAM:
            throw std::runtime_error("Unexpected node in generateCode.");
    }

    return assembly;
}

std::string generateFunctionCode(const FunctionDefinitionNode* func_node) {
    std::string assembly;
    stack_offsets.clear();
    current_stack_offset = 0;

    assembly += func_node->name + ":\n";
    assembly += "  push rbp\n";
    assembly += "  mov rbp, rsp\n";

    // Assign offsets to parameters
    // Parameters are passed on the stack, starting at rbp + 16 (return address + old rbp)
    int param_offset = 16;
    for (const auto& param : func_node->parameters) {
        stack_offsets[param->name] = param_offset;
        param_offset += 8; // Assuming 8-byte parameters (qword)
    }

    // Process function body statements
    for (const auto& stmt : func_node->body_statements) {
        assembly += generateCode(stmt.get());
    }

    // Function epilogue
    if (func_node->name == "main") {
        assembly += ".main_epilogue:\n";
        // For main, the return value is in rax, which is moved to rdi for syscall 60 (exit)
    }

    assembly += "  mov rsp, rbp\n"; // Restore stack pointer
    assembly += "  pop rbp\n";      // Restore base pointer
    assembly += "  ret\n\n";        // Return from function
    return assembly;
}

std::string escapeString(const std::string& raw) {
    std::string out = "\"";
    for (char c : raw) {
        if (c == '\\') out += "\\";
        else if (c == '"') out += "\"";
        else if (c == '\n') out += "\n";
        else out += c;
    }
    out += "\"";
    return out;
}