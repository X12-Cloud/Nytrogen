#include <sstream>
#include <stdexcept>

#include "instruction_set.hpp"

void InstructionSet::emit_adv(int size, const TypeNode* type, const std::string& base_reg,
                             int offset, const std::string& src_val) {
    bool is_fp = isAFloatingPoint(const_cast<TypeNode*>(type));
    std::string size_prefix = get_size_prefix(size);

    if (is_fp) {
        std::string instr = (size == 4) ? "vmovss" : "vmovsd";
        out << "    " << instr << " [" << base_reg << " + " << offset << "], " << src_val
            << std::endl;
    } else {
        std::string final_src = src_val;

        if (src_val == "rbx") {
            if (size == 1) {
                final_src = "bl";
            } else if (size == 4) {
                final_src = "ebx";
            }
        } else if (src_val == "rax") {
            if (size == 1) {
                final_src = "al";
            } else if (size == 4) {
                final_src = "eax";
            }
        }

        out << "    mov " << size_prefix << " [" << base_reg << " + " << offset << "], "
            << final_src << std::endl;
    }
}

void InstructionSet::load_adv(int size, const TypeNode* type, const std::string& dest_reg,
                             const std::string base_reg, int offset) {
    bool is_fp = isAFloatingPoint(type);
    std::string off_str = std::to_string(offset);

    if (is_fp) {
        if (size == 4) {
            emit("vmovss", dest_reg, "[" + base_reg + " + " + off_str + "]");
        } else {
            emit("vmovsd", dest_reg, "[" + base_reg + "+" + off_str + "]");
        }
    } else {
        if (size == 1) {
            emit("movsx", dest_reg, "byte [" + base_reg + " + " + off_str + "]");
        } else if (size == 4) {
            emit("movsx", dest_reg, "dword [" + base_reg + " + " + off_str + "]");
        } else {
            emit("mov", dest_reg, "[" + base_reg + " + " + off_str + "]");
        }
    }
}

void InstructionSet::load_from_address(int size, const std::string& reg) {
    if (size == 4) {
        emit("movsx", "rax", "dword [" + reg + "]");
    } else if (size == 1) {
        emit("movsx", "rax", "byte [" + reg + "]");
    } else {
        emit("mov", "rax", "[" + reg + "]");
    }
}

void InstructionSet::emit_print(int size, const std::shared_ptr<TypeNode>& type) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());

    if (isAFloatingPoint(type.get())) {
        if (size == 4) {
            emit("cvtss2sd", "xmm0", "xmm0");
        }
        emit("lea", "rdi", "[rel _print_float_format]");
        emit("mov", "rax", "1");
    } else if ((prim != nullptr) && prim->primitive_type == Token::KEYWORD_STRING) {
        emit("mov", "rsi", "rax");
        emit("lea", "rdi", "[rel _print_str_format]");
        emit("xor", "rax", "rax");
    } else if ((prim != nullptr) && prim->primitive_type == Token::KEYWORD_CHAR) {
        emit("mov", "rsi", "rax");
        emit("lea", "rdi", "[rel _print_char_format]");
        emit("xor", "rax", "rax");
    } else {
        emit("mov", "rsi", "rax");
        emit("lea", "rdi", "[rel _print_int_format]");
        emit("xor", "rax", "rax");
    }
    call_external("printf");
}

void InstructionSet::emit_print_int(const std::string& reg) {
    emit("mov", "rsi", reg);
    emit("lea", "rdi", "[rel _print_int_format]");
    emit("xor", "rax", "rax");
    call_external("printf");
}
