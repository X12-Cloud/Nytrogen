#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

// Function to read the entire content of a file into a string
std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file for reading: " << filepath << std::endl;
        return "";
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

// Function to process a single file, handling includes
void processFile(const std::string& input_filepath, std::ostream& output_stream) {
    std::ifstream input_file(input_filepath);
    if (!input_file.is_open()) {
        std::cerr << "Error: Could not open input file: " << input_filepath << std::endl;
        return;
    }

    std::string line;
    while (std::getline(input_file, line)) {
        if (line.find("include ") == 0) {
            std::string include_path;
            fs::path current_file_path(input_filepath);
            fs::path included_file_path;

            size_t start_delim;
            size_t end_delim;

            if ((start_delim = line.find('<')) != std::string::npos && (end_delim = line.find('>')) != std::string::npos && start_delim == (line.find("include ") + 8)) {
                // Handle <...> inclusion
                include_path = line.substr(start_delim + 1, end_delim - start_delim - 1);
                // Prepend "std/" to the path. Assuming "std" is at the root of the project
                // The preprocessor currently runs from the project root.
                // We need to resolve relative to the project root, not the current file's parent path.
                included_file_path = fs::path("std/") / include_path; // Use fs::path for concatenation
            } else if ((start_delim = line.find('"')) != std::string::npos && (end_delim = line.find('"', start_delim + 1)) != std::string::npos && start_delim == (line.find("include ") + 8)) {
                // Handle "..." inclusion
                include_path = line.substr(start_delim + 1, end_delim - start_delim - 1);
                // Resolve relative to the file that is including it.
                included_file_path = current_file_path.parent_path() / include_path;
            } else {
                std::cerr << "Preprocessor Error: Invalid include directive: " << line << std::endl;
                output_stream << line << std::endl; // Output original line to not break compilation
                continue;
            }
            
            std::string included_content = readFileContent(included_file_path.string());
            if (!included_content.empty()) {
                output_stream << included_content << std::endl;
            } else {
                std::cerr << "Preprocessor Warning: Could not find included file: " << included_file_path.string() << std::endl;
            }
        } else {
            output_stream << line << std::endl;
        }
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>" << std::endl;
        return 1;
    }

    std::string input_filepath = argv[1];

    processFile(input_filepath, std::cout);

    return 0;
}
