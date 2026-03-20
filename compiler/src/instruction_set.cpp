#include "code_generator.hpp"
#include <stdexcept>
#include <sstream>

void CodeGenerator::emit(std::string instruction, std::string dest, std::string data) {
    out << instruction << dest << ", " << data << std::endl;
}

void CodeGenerator::emit_binary_op() {
}
