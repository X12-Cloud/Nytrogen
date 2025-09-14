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
        if (line.find("include \"" ) == 0) {
            size_t start_quote = line.find('"');
            size_t end_quote = line.find('"', start_quote + 1);
            if (start_quote != std::string::npos && end_quote != std::string::npos) {
                std::string include_path = line.substr(start_quote + 1, end_quote - start_quote - 1);
                
                // To handle relative paths correctly, we resolve the path of the included file
                // relative to the file that is including it.
                fs::path current_file_path(input_filepath);
                fs::path included_file_path = current_file_path.parent_path() / include_path;

                std::string included_content = readFileContent(included_file_path.string());
                if (!included_content.empty()) {
                    output_stream << included_content << std::endl;
                }
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
