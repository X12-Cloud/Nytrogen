#include <cmath>
#include <sstream>
#include <stdexcept>

#include "instruction_set.hpp"

std::string InstructionSet::get_size_prefix(int size) {
    switch (size) {
        case 1:
            return "byte";
        case 4:
            return "dword";
        case 8:
            return "qword";
        default:
            return "qword";
    }
}

// Basic Emitters
void InstructionSet::emit(const std::string& instr) {
    instructions.push_back({instr, {}});
}

void InstructionSet::emit(const std::string& instr, const std::string reg) {
    instructions.push_back({instr, {reg}});
}

void InstructionSet::emit(const std::string& instr, const std::string& dest,
                          const std::string& src) {
    instructions.push_back({instr, {dest, src}});
}

// Sections and Symbols
void InstructionSet::global(const std::string& name) {
    instructions.push_back({"global", {name}});
}

void InstructionSet::section(const std::string& section_name) {
    instructions.push_back({"section", {section_name}});
}

void InstructionSet::extern_sym(const std::string& name) {
    instructions.push_back({"extern", {name}});
}

void InstructionSet::label(const std::string& label_name) {
    instructions.push_back({label_name + ":", {}});
}

void InstructionSet::label_local(const std::string& name) {
    instructions.push_back({name + ":", {}});
}

// Memory Operations
void InstructionSet::mov_indirect(const std::string& base, int offset, const std::string& src) {
    std::string addr =
        "[" + base + (offset >= 0 ? " + " : " - ") + std::to_string(std::abs(offset)) + "]";
    emit("mov", addr, src);
}

void InstructionSet::emit_mem_rel(const std::string& instr, const std::string& label,
                                  const std::string& reg) {
    emit(instr, "[rel " + label + "]", reg);
}

// Stack Operations
void InstructionSet::push(const std::string& reg) {
    instructions.push_back({"push", {reg}});
    current_stack_depth += 8;
}

void InstructionSet::pop(const std::string& reg) {
    instructions.push_back({"pop", {reg}});
    current_stack_depth -= 8;
}

// Specialized Emitters
void InstructionSet::emit_lea_stack(const std::string& dest, int offset) {
    std::string operand =
        std::string("[rbp ") + (offset >= 0 ? "+ " : "- ") + std::to_string(std::abs(offset)) + "]";
    emit("lea", dest, operand);
}

void InstructionSet::emit_load_constant(const std::string& instr, const std::string& dest,
                                        const std::string& label) {
    emit(instr, dest, "[rel " + label + "]");
}

void InstructionSet::emit_dereference(const std::string& dest, const std::string& src) {
    emit("mov", dest, "[" + src + "]");
}

void InstructionSet::syscall(int code) {
    emit("mov", "rax", std::to_string(code));
    emit("syscall");
}

void InstructionSet::emit_data_entry(const std::string& label, const std::string& type,
                                     const std::string& value) {
    instructions.push_back({label + ": " + type, {value}});
}

bool InstructionSet::isAFloatingPoint(const TypeNode* type) {
    if (!type)
        return false;
    auto prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}

// Call logic with alignment
void InstructionSet::call_external(const std::string& func_name) {
    bool misaligned = (current_stack_depth % 16 != 0);
    if (misaligned)
        emit("sub", "rsp", "8");
    emit("call", func_name);
    if (misaligned)
        emit("add", "rsp", "8");
}

void InstructionSet::flush_to_file() {
    for (size_t i = 0; i < instructions.size(); ++i) {
        const auto& instr = instructions[i];
        if (instr.mnemonic.empty())
            continue;

        // Handle Labels
        if (instr.mnemonic.back() == ':') {
            out << "\n" << instr.mnemonic;
            if (i + 1 < instructions.size() &&
                (instructions[i + 1].mnemonic == "db" || instructions[i + 1].mnemonic == "dw" ||
                 instructions[i + 1].mnemonic == "dd" || instructions[i + 1].mnemonic == "dq")) {
                out << " " << instructions[i + 1].mnemonic;
                for (size_t j = 0; j < instructions[i + 1].operands.size(); ++j) {
                    out << (j == 0 ? " " : ", ") << instructions[i + 1].operands[j];
                }
                out << "\n";
                i++;
            } else {
                out << "\n";
            }
        } else if (instr.mnemonic == "section") {
            out << "\nsection " << instr.operands[0] << "\n";
        } else {
            out << "    " << instr.mnemonic;
            for (size_t j = 0; j < instr.operands.size(); ++j) {
                out << (j == 0 ? " " : ", ") << instr.operands[j];
            }
            out << "\n";
        }
    }
    instructions.clear();
}
