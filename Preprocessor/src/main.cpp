#include <iostream>

#include "file_parser.hpp"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input_file>\n";
        return 1;
    }

    FileParser preprocessor;
    preprocessor.parse(argv[1], std::cout);

    return 0;
}
