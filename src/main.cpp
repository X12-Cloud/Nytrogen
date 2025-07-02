#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <stdexcept>
#include <algorithm>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

std::map<std::string, int> stack_offsets;
int current_stack_offset = 0;

std::string generateCode(const ASTNode* node);
std::string generateFunctionCode(const FunctionDefinitionNode* func_node);

std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    return buffer.str();
}

int main(int argc, char* argv[]) {
    std::cout << "Nytrogen Compiler (Arch Linux)" << std::endl;

    if (argc < 2) {
        std::cerr << "Error: No source file provided. Usage: ./Nytro <source_file>" << std::endl;
        return 2;
    }

    std::string input_filepath = argv[1];
    std::string output_asm_filename = "out.asm";

    std::string ext = input_filepath.substr(input_filepath.find_last_of(".") + 1);
    if (ext != "ny" && ext != "nyt") {
        std::cerr << "Error: Input file must have .ny or .nyt extension (found: ." << ext << ")" << std::endl;
        return 3;
    }

    std::string sourceCode = readFileContent(input_filepath);
    if (sourceCode.empty()) {
        return 2;
    }

    std::cout << "\n--- Processing Source File: " << input_filepath << " ---\n" << std::endl;

    std::vector<Token> tokens = tokenize(sourceCode);
    Parser parser(std::move(tokens));
    std::unique_ptr<ProgramNode> ast_root = parser.parse();

    if (!ast_root) {
        std::cerr << "AST generation failed during parsing. Exiting." << std::endl;
        return 1;
    }

    std::ofstream asm_file(output_asm_filename, std::ios::binary);
    if (!asm_file.is_open()) {
        std::cerr << "Error: Could not create output assembly file: " << output_asm_filename << std::endl;
        return 2;
    }

    // NEW ADDITION: Data section for printf format string
    asm_file << "section .data\n";
    asm_file << "  _print_int_format db \"%d\", 10, 0 ; Format string for printing integers, including newline\n";
    asm_file << "\n"; // Add a newline for separation

    asm_file << "section .text\n";
    asm_file << "global _start\n";
    asm_file << "extern printf ; NEW ADDITION: Declare printf as an external function\n";

    for (const auto& func : ast_root->functions) {
        asm_file << generateFunctionCode(func.get());
    }

    asm_file << "_start:\n";
    asm_file << "  call main\n";
    asm_file << "  mov rdi, rax\n";
    asm_file << "  mov rax, 60\n";
    asm_file << "  syscall\n";

    asm_file.close();
    std::cout << "Successfully generated assembly code to '" << output_asm_filename << "'" << std::endl;

    return 0;
}

std::string generateCode(const ASTNode* node) {
    if (!node) {
        throw std::runtime_error("Code generation error: Encountered a null AST node.");
    }

    std::string assembly = "";

    switch (node->node_type) {
        case ASTNode::NodeType::PROGRAM: {
            throw std::runtime_error("Code generation error: PROGRAM node should be traversed in main.");
        }
        case ASTNode::NodeType::INTEGER_LITERAL_EXPRESSION: {
            const IntegerLiteralExpressionNode* int_node = static_cast<const IntegerLiteralExpressionNode*>(node);
            assembly += "  mov rax, " + std::to_string(int_node->value) + "\n";
            break;
        }
        case ASTNode::NodeType::RETURN_STATEMENT: {
            const ReturnStatementNode* return_node = static_cast<const ReturnStatementNode*>(node);
            assembly += generateCode(return_node->expression.get());
            assembly += "  jmp .main_epilogue\n";
            break;
        }
        case ASTNode::NodeType::VARIABLE_DECLARATION: {
            break;
        }
        case ASTNode::NodeType::VARIABLE_ASSIGNMENT: {
            const VariableAssignmentNode* assign_node = static_cast<const VariableAssignmentNode*>(node);
            assembly += generateCode(assign_node->expression.get());
            int offset = stack_offsets.at(assign_node->name);
            assembly += "  mov qword [rbp + " + std::to_string(offset) + "], rax\n";
            break;
        }
        case ASTNode::NodeType::VARIABLE_REFERENCE: {
            const VariableReferenceNode* ref_node = static_cast<const VariableReferenceNode*>(node);
            int offset = stack_offsets.at(ref_node->name);
            assembly += "  mov rax, qword [rbp + " + std::to_string(offset) + "]\n";
            break;
        }
        case ASTNode::NodeType::BINARY_OPERATION_EXPRESSION: {
            const BinaryOperationExpressionNode* bin_op_node = static_cast<const BinaryOperationExpressionNode*>(node);

            assembly += generateCode(bin_op_node->left.get());
            assembly += "  push rax\n";

            assembly += generateCode(bin_op_node->right.get());

            assembly += "  pop rbx\n";

            switch (bin_op_node->op_type) {
                case Token::PLUS:
                    assembly += "  add rbx, rax\n";
                    assembly += "  mov rax, rbx\n";
                    break;
                case Token::MINUS:
                    assembly += "  sub rbx, rax\n";
                    assembly += "  mov rax, rbx\n";
                    break;
                case Token::STAR:
                    assembly += "  imul rbx\n";
                    break;
                case Token::SLASH:
                    assembly += "  mov rcx, rax\n";
                    assembly += "  mov rax, rbx\n";
                    assembly += "  mov rbx, rcx\n";
                    assembly += "  xor rdx, rdx\n";
                    assembly += "  idiv rbx\n";
                    break;
                default:
                    throw std::runtime_error("Code generation error: Unhandled binary operator type.");
            }
            break;
        }
        case ASTNode::NodeType::PRINT_STATEMENT: {
            const PrintStatementNode* print_node = static_cast<const PrintStatementNode*>(node);
            assembly += generateCode(print_node->expression.get());
            assembly += "  mov rsi, rax\n";
            assembly += "  lea rdi, [rel _print_int_format]\n";
            assembly += "  xor rax, rax\n";
            assembly += "  call printf\n";
            break;
        }
        case ASTNode::NodeType::FUNCTION_DEFINITION: {
            throw std::runtime_error("Code generation error: FUNCTION_DEFINITION node should be handled by generateFunctionCode directly.");
        }
        default: {
            throw std::runtime_error("Code generation error: Unhandled AST node type: " +
                                     std::to_string(static_cast<int>(node->node_type)) +
                                     " (at line " + std::to_string(node->line) +
                                     ", column " + std::to_string(node->column) + ")");
        }
    }
    return assembly;
}

std::string generateFunctionCode(const FunctionDefinitionNode* func_node) {
    std::string assembly = "";

    stack_offsets.clear();
    current_stack_offset = 0;

    assembly += func_node->name + ":\n";

    assembly += "  push rbp\n";
    assembly += "  mov rbp, rsp\n";

    int total_local_vars_size = 0;
    std::vector<std::string> declared_vars_in_func;
    for (const auto& stmt : func_node->body_statements) {
        if (stmt->node_type == ASTNode::NodeType::VARIABLE_DECLARATION) {
            const VariableDeclarationNode* decl_node = static_cast<const VariableDeclarationNode*>(stmt.get());
            current_stack_offset -= 8;
            stack_offsets[decl_node->name] = current_stack_offset;
            declared_vars_in_func.push_back(decl_node->name);
            total_local_vars_size += 8;
        }
    }

    if (total_local_vars_size % 16 != 0) {
        total_local_vars_size = (total_local_vars_size + 15) / 16 * 16;
    }
    if (total_local_vars_size > 0) {
        assembly += "  sub rsp, " + std::to_string(total_local_vars_size) + "\n";
    }

    for (const auto& var_name : declared_vars_in_func) {
        int offset = stack_offsets.at(var_name);
        assembly += "  mov qword [rbp + " + std::to_string(offset) + "], 0\n";
    }

    for (const auto& statement : func_node->body_statements) {
        if (statement->node_type == ASTNode::NodeType::VARIABLE_DECLARATION) {
            continue;
        }
        assembly += generateCode(statement.get());
    }

    if (func_node->name == "main") {
        assembly += ".main_epilogue:\n";
        assembly += "  mov rsp, rbp\n";
        assembly += "  pop rbp\n";
        assembly += "  ret\n";
    } else {
        assembly += "  mov rsp, rbp\n";
        assembly += "  pop rbp\n";
        assembly += "  ret\n";
    }
    assembly += "\n";

    return assembly;
}

