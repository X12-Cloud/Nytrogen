#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <cctype>
#include <map>
#include <stdexcept>

#include "lexer.hpp"
#include "parser.hpp"
#include "ast.hpp"

std::map<std::string, int> stack_offsets;
int current_stack_offset = 0;

std::string generateCode(const ASTNode* node);

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
    input_filepath = argv[1];

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

    // x86-64 Assembly setup
    asm_file << "section .text\n";
    asm_file << "global _start\n";
    asm_file << "_start:\n";
    asm_file << "  push rbp\n";
    asm_file << "  mov rbp, rsp\n";

    for (const auto& statement : ast_root->statements) {
        asm_file << generateCode(statement.get());
    }

    asm_file << "  mov rax, 60\n";
    asm_file << "  xor rdi, rdi\n";
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
            assembly += "  mov rdi, rax\n";
            assembly += "  mov rax, 60\n";
            assembly += "  syscall\n";
            break;
        }
        case ASTNode::NodeType::VARIABLE_DECLARATION: {
            const VariableDeclarationNode* decl_node = static_cast<const VariableDeclarationNode*>(node);
            current_stack_offset -= 8;
            stack_offsets[decl_node->name] = current_stack_offset;
            assembly += "  mov qword [rbp + " + std::to_string(current_stack_offset) + "], 0\n";
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
        default: {
            throw std::runtime_error("Code generation error: Unhandled AST node type: " +
                                     std::to_string(static_cast<int>(node->node_type)) +
                                     " (at line " + std::to_string(node->line) +
                                     ", column " + std::to_string(node->column) + ")");
        }
    }
    return assembly;
}
