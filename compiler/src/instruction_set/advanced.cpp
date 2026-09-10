#include <sstream>
#include <stdexcept>
#include <cstring>
#include <cstdint>
#include <algorithm>

#include "instruction_set.hpp"

uint64_t string_to_imm64(std::string_view sv) {
    uint64_t val = 0;
    size_t bytes_to_copy = std::min(sv.size(), sizeof(uint64_t));
    std::memcpy(&val, sv.data(), bytes_to_copy);
    return val;
}

void InstructionSet::emit_adv(int size, const TypeNode* type, const std::string& base_vreg,
                              int offset, const std::string& src_vreg) {
    bool is_fp = isAFloatingPoint(type);
    auto* prim = dynamic_cast<const PrimitiveTypeNode*>(type);
    bool is_complex = (prim && prim->primitive_type == Token::KEYWORD_COMPLEX);

    std::string addr_raw = "[" + base_vreg + " + " + std::to_string(offset) + "]";

    if (is_complex) {
        emit("vmovupd", "oword " + addr_raw, src_vreg);
    } else if (is_fp) {
        std::string prefix = (size == 4) ? "dword " : "qword ";
        emit(size == 4 ? "vmovss" : "vmovsd", prefix + addr_raw, src_vreg);
    } else {
        std::string prefix = get_size_prefix(size) + " ";
        emit("mov", prefix + addr_raw, src_vreg);
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
            emit("movsxd", dest_vreg, "dword " + addr);
        else if (size == 8)
            emit("mov", dest_vreg, "qword " + addr);
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
                                const std::string& src_vreg, const std::string& outs) {
    emit_print_internal(size, type, src_vreg, false, outs);
}

void InstructionSet::emit_print_raw(int size, const std::shared_ptr<TypeNode>& type,
                                    const std::string& src_vreg, const std::string& outs) {
    emit_print_internal(size, type, src_vreg, true, outs);
}

// Helper to consolidate logic
void InstructionSet::emit_print_internal(int size, const std::shared_ptr<TypeNode>& type,
                                         const std::string& src_vreg, bool is_raw, 
                                         const std::string& outs_vreg) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_fp = isAFloatingPoint(type.get());

    // Qubits
    if (prim && prim->primitive_type == Token::KEYWORD_QUBIT) {
        emit("mov", "rdi", outs_vreg);
        emit("mov", "rsi", src_vreg);
        call_external("print_q");
        return;
    }

    // Complex literals
    if (prim && prim->primitive_type == Token::KEYWORD_COMPLEX) {
        emit("mov", "rax", src_vreg);
        emit("vmovsd", "xmm0", "[rax]");
        emit("vmovsd", "xmm1", "[rax + 8]");
        emit("mov", "rdi", outs_vreg);
        call_external("ny_print_complex");
        return;
    }

    if (is_fp) {
        emit("mov", "rdi", outs_vreg);
        if (size == 4) {
            emit("vcvtss2sd", "xmm0", src_vreg + ", " + src_vreg);
        } else {
            emit("vmovsd", "xmm0", src_vreg);
        }
        call_external(is_raw ? "ny_print_float_raw" : "ny_print_float");
    } else {
        emit("mov", "rdi", outs_vreg);
        emit("mov", "rsi", src_vreg);

        std::string func;
        if (prim && prim->primitive_type == Token::KEYWORD_STRING) {
            func = "ny_print_string";
        } else if (prim && prim->primitive_type == Token::KEYWORD_CHAR) {
            func = "ny_print_char";
        } else if (prim && prim->primitive_type == Token::KEYWORD_BOOL) {
            func = "ny_print_bool";
        } else {
            func = "ny_print_int";
        }
        std::string_view func_type = std::string_view(func).substr(9);
        std::stringstream type;
        type << std::hex << "0x" << string_to_imm64(func_type);

        emit("mov", "rax", type.str());
        push("rax");
        push("rax");
        emit("mov", "rdx", "rsp");
        emit("mov", "rcx", std::to_string((int)is_raw));

        call_external("ny_print");
        emit("add", "rsp", "16");
    }
}

void InstructionSet::emit_print_int(const std::string& src_vreg) {
    emit("mov", "rsi", src_vreg);
    emit("lea", "rdi", "[rel _print_int_format]");
    emit("xor", "rax", "rax");
    call_external("printf");
}
