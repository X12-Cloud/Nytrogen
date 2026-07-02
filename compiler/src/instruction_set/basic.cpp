#include <sstream>
#include <stdexcept>
#include <cmath>

#include "instruction_set.hpp"

std::string InstructionSet::get_size_prefix(int size) {
    switch(size) {
        case 1: return "byte";
        case 4: return "dword";
        default: return "qword";
    }
}

void InstructionSet::emit(const std::string& instr) { 
//    out << "    " << instr << std::endl;
    instructions.push_back({instr, {}}); // Experimental: pushing to the instructions vector for the reg alloc
}
void InstructionSet::emit(const std::string& instr, const std::string reg) {
//    out << "    " << instr << " " << reg << std::endl;
    instructions.push_back({instr, {reg}});
}
void InstructionSet::emit(const std::string& instr, const std::string& dest, const std::string& src) {
//    out << "    " << instr << " " << dest << ", " << src << std::endl;
    instructions.push_back({instr, {dest, src}});
}

void InstructionSet::global(const std::string& name) { emit("global", name); }
void InstructionSet::section(const std::string& section_name) {
//    out << "\nsection " << section_name << std::endl;
    instructions.push_back({"section", {section_name}});
}

void InstructionSet::extern_sym(const std::string& name) { emit("extern", name); }
void InstructionSet::label(const std::string& label) {
//    out << label << ":" << std::endl;
    instructions.push_back({label + ":", {}});
}
void InstructionSet::label_local(const std::string& name) {
//    out << name << ":" << std::endl;
    instructions.push_back({name + ":", {}});
}
void InstructionSet::mov_indirect(const std::string& base, int offset, const std::string& src) {
//    out << "    mov [rbp + " << offset << "], " << src << std::endl;
    instructions.push_back({"mov", {"[rbp + " + std::to_string(offset) + "]", src}});
}
void InstructionSet::write_raw(const std::string& data) {
//    out << data;
    //instructions.push_back({data, {}});
}
void InstructionSet::emit_mem_rel(const std::string& instr, const std::string& label, const std::string& reg) {
//    out << "    " << instr << " [rel " << label << "], " << reg << std::endl;
    instructions.push_back({instr, {"[rel " + label + "]", reg}});
}
void InstructionSet::syscall(int code) {
    emit("mov", "rax", std::to_string(code));
    emit("syscall");
}

void InstructionSet::push(const std::string& reg) {
//    out << "    " << "push" << " " << reg << std::endl;
    instructions.push_back({"push", {reg}});
    current_stack_depth += 8;
}

void InstructionSet::pop(const std::string& reg) {
//    out << "    " << "pop" << " " << reg << std::endl;
    instructions.push_back({"pop", {reg}});
    current_stack_depth -= 8;
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

void InstructionSet::emit_lea_stack(int offset) {
    std::string operand = std::string("[rbp ") + (offset >= 0 ? "+ " : "- ") + std::to_string(std::abs(offset)) + "]";
    emit("lea", "rax", operand);
}
void InstructionSet::emit_load_constant(const std::string& instr, const std::string& label) {
    emit(instr, "xmm0", "[rel " + label + "]");
}
void InstructionSet::emit_dereference() {
    emit("mov", "rax", "[rax]");
}
void InstructionSet::emit_data_entry(const std::string& label, const std::string& type, const std::string& value) {
//    out << "    " << label << " " << type << " " << value << std::endl;
    instructions.push_back({label + ": " + type, {value}});
}

bool InstructionSet::isAFloatingPoint(const TypeNode* type) {
    if (type == nullptr) {
        return false;
    }
    auto prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}
