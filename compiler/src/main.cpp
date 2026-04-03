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

#include "code_generator.hpp"

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
    // std::cout << "Nytrogen Compiler (Arch Linux)\n";

    if (argc < 2) {
        std::cerr << "Error: No source file provided. Usage: ./nytro-c <source_file> [output_dir]\n";
        return 2;
    }

    std::string input_filepath = argv[1];
    std::string output_asm_filename = "out.asm";
    if (argc > 2) {
        output_asm_filename = argv[2];
    } 
    bool debug_mode = false;
    bool verbose = false;
    bool is_entry = false;
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "-debug") {
            debug_mode = true;
        } else if (std::string(argv[i]) == "-verbose") {
            verbose = true;
        } else if (std::string(argv[i]) == "-entry") {
            is_entry = true;
        }
    }

    std::string ext = input_filepath.substr(input_filepath.find_last_of(".") + 1);
    if (ext != "ny" && ext != "nyt") {
        std::cerr << "Error: Input file must have .ny or .nyt extension (found: ." << ext << ")\n";
        return 3;
    }

    std::string sourceCode = readFileContent(input_filepath);
    if (sourceCode.empty()) return 2;

    if (verbose) std::cout << "\n--- Processing Source File: " << input_filepath << " ---\n\n";

    std::vector<Token> tokens = tokenize(sourceCode);
    Parser parser(std::move(tokens));
    parser.getSymbolTable().setDebugMode(debug_mode);
    
    std::unique_ptr<ProgramNode> ast_root = parser.parse();

    if (!ast_root) {
        std::cerr << "AST generation failed during parsing. Exiting.\n";
        return 1;
    }

    // Perform semantic analysis
    SemanticAnalyzer semanticAnalyzer(ast_root, parser.getSymbolTable());
    semanticAnalyzer.setIsEntryPoint(is_entry);
    semanticAnalyzer.analyze();

    // Generate code
    CodeGenerator codeGenerator(ast_root, semanticAnalyzer.getSymbolTable());
    codeGenerator.generate(output_asm_filename, is_entry);

    if (verbose) std::cout << "Successfully generated assembly to '" << output_asm_filename << "'\n";

    if (verbose) ast_root->dump();

    return 0;
}

