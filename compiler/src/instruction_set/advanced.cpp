#include <sstream>
#include <stdexcept>

#include "instruction_set.hpp"

void InstructionSet::emit_adv(int size, const TypeNode* type, const std::string& base_vreg,
                              int offset, const std::string& src_vreg) {
    bool is_fp = isAFloatingPoint(type);
    std::string size_prefix = get_size_prefix(size);
    std::string addr = size_prefix + " [" + base_vreg + " + " + std::to_string(offset) + "]";

    if (is_fp) {
        emit(size == 4 ? "vmovss" : "vmovsd", addr, src_vreg);
    } else {
        emit("mov", addr, src_vreg);
    }
}

void InstructionSet::load_adv(int size, const TypeNode* type, const std::string& dest_vreg,
                              const std::string& base_vreg, int offset) {
    bool is_fp = isAFloatingPoint(type);
    std::string addr = "[" + base_vreg + " + " + std::to_string(offset) + "]";

    if (is_fp) {
        emit(size == 4 ? "vmovss" : "vmovsd", dest_vreg, addr);
    } else {
        if (size == 1)
            emit("movsx", dest_vreg, "byte " + addr);
        else if (size == 4)
            emit("movsxd", dest_vreg, "dword " + addr);  // FIX HERE
        else
            emit("mov", dest_vreg, addr);
    }
}

void InstructionSet::load_from_address(int size, const TypeNode* type, const std::string& dest_vreg,
                                       const std::string& addr_vreg) {
    bool is_fp = isAFloatingPoint(type);
    std::string prefix = get_size_prefix(size);
    std::string addr = "[" + addr_vreg + "]";

    if (is_fp) {
        emit(size == 4 ? "vmovss" : "vmovsd", dest_vreg, addr);
    } else {
        if (size == 1)
            emit("movsx", dest_vreg, "byte " + addr);
        else if (size == 4)
            emit("movsx", dest_vreg, "dword " + addr);
        else
            emit("mov", dest_vreg, addr);
    }
}

void InstructionSet::emit_print(int size, const std::shared_ptr<TypeNode>& type,
                                const std::string& src_vreg) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_fp = isAFloatingPoint(type.get());

    if (is_fp) {
        if (size == 4) {
            emit("vcvtss2sd", "xmm0", src_vreg + ", " + src_vreg);
        } else {
            emit("vmovsd", "xmm0", src_vreg);
        }
        emit("lea", "rdi", "[rel _print_float_format]");
        emit("mov", "rax", "1");
    } else {
        emit("mov", "rsi", src_vreg);

        if (prim && prim->primitive_type == Token::KEYWORD_STRING) {
            emit("lea", "rdi", "[rel _print_str_format]");
        } else if (prim && prim->primitive_type == Token::KEYWORD_CHAR) {
            emit("lea", "rdi", "[rel _print_char_format]");
        } else {
            emit("lea", "rdi", "[rel _print_int_format]");
        }
        emit("xor", "rax", "rax");
    }
    call_external("printf");
}

void InstructionSet::emit_print_raw(int size, const std::shared_ptr<TypeNode>& type,
                                    const std::string& src_vreg) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_fp = isAFloatingPoint(type.get());

    if (is_fp) {
        if (size == 4) {
            emit("vcvtss2sd", "xmm0", src_vreg + ", " + src_vreg);
        } else {
            emit("vmovsd", "xmm0", src_vreg);
        }
        emit("lea", "rdi", "[rel _print_float_raw_format]");
        emit("mov", "rax", "1");
    } else {
        emit("mov", "rsi", src_vreg);

        if (prim && prim->primitive_type == Token::KEYWORD_STRING) {
            emit("lea", "rdi", "[rel _print_raw_format]");
        } else if (prim && prim->primitive_type == Token::KEYWORD_CHAR) {
            emit("lea", "rdi", "[rel _print_char_raw_format]");
        } else {
            emit("lea", "rdi", "[rel _print_int_raw_format]");
        }
        emit("xor", "rax", "rax");
    }
    call_external("printf");
}

void InstructionSet::emit_print_int(const std::string& src_vreg) {
    emit("mov", "rsi", src_vreg);
    emit("lea", "rdi", "[rel _print_int_format]");
    emit("xor", "rax", "rax");
    call_external("printf");
}
