#include <sstream>
#include <stdexcept>

#include "instruction_set.hpp"

std::string InstructionSet::get_size_prefix(int size) {
    switch(size) {
        case 1: return "byte";
        case 4: return "dword";
        default: return "qword";
    }
}

void InstructionSet::emit(const std::string& instr) { out << "    " << instr << std::endl; }

void InstructionSet::emit(const std::string& instr, const std::string reg) {
    out << "    " << instr << " " << reg << std::endl;
}

void InstructionSet::emit(const std::string& instr, const std::string& dest,
                         const std::string& src) {
    out << "    " << instr << " " << dest << ", " << src << std::endl;
}

void InstructionSet::push(const std::string& reg) {
    out << "    " << "push" << " " << reg << std::endl;
    current_stack_depth += 8;
}

void InstructionSet::call_external(const std::string& func_name) {
    bool misaligned = (current_stack_depth % 16 != 0);

    if (misaligned) {
        emit("sub", "rsp", "8");
    }

    emit("call", func_name);

    if (misaligned) {
        emit("add", "rsp", "8");
    }
}

bool InstructionSet::isAFloatingPoint(const TypeNode* type) {
    if (type == nullptr) {
        return false;
    }
    auto prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}
