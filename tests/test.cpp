#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>

std::string readFileContent(const std::string& filepath) {
    std::ifstream file(filepath); // Open the file
    if (!file.is_open()) {
        std::cerr << "Error: Could not open file " << filepath << std::endl;
        return ""; // Return empty string on failure
    }

    // Read the entire file content into a string stream
    std::stringstream buffer;
    buffer << file.rdbuf(); // rdbuf() returns a pointer to the file's streambuf
    file.close();           // Close the file stream

    return buffer.str();    // Return the content as a string
}

int main(int argc, char* argv[]) {
    std::cout << "Hello from Nytrogen Compiler (Arch Linux)!" << std::endl;

    // This section simulates the very beginning of argument parsing,
    // which was a key part of the video's first segment.
    if (argc > 1) {
        std::cout << "Arguments received:" << std::endl;
        for (int i = 1; i < argc; ++i) {
            std::cout << "  " << argv[i] << std::endl;
        }
    } else {
        std::cout << "No arguments provided. Usage: ./Nytro <source_file>" << std::endl;
    }

    return 0;
}