#include <sstream>
#include <stdexcept>

#include "code_generator.hpp"

std::string CodeGenerator::reg_to_str(RegisterAllocator::RegID reg, const TypeNode* type) {
    int size = getTypeSize(type);
    if (reg == RegisterAllocator::RegID::NONE) {
        return (size == 1) ? "al" : (size == 4) ? "eax" : "rax";
    }
    return allocator.get_name(reg, size);
}

void CodeGenerator::emit(const std::string& instr) { out << "    " << instr << std::endl; }

void CodeGenerator::emit(const std::string& instr, const std::string reg) {
    out << "    " << instr << " " << reg << std::endl;
}

void CodeGenerator::emit(const std::string& instr, const std::string& dest,
                         const std::string& src) {
    out << "    " << instr << " " << dest << ", " << src << std::endl;
}

void CodeGenerator::emit_adv(const std::shared_ptr<TypeNode>& type, const std::string& base_reg,
                             int offset, const std::string& src_val) {
    int size = getTypeSize(type.get());
    bool is_fp = isFloatingPoint(type);
    
    // Determine size prefix for NASM
    std::string size_prefix;
    if (size == 1) size_prefix = "byte";
    else if (size == 4) size_prefix = "dword";
    else size_prefix = "qword"; // Default for 8-byte/pointers

    if (is_fp) {
        std::string instr = (size == 4) ? "vmovss" : "vmovsd";
        // Force the size prefix here too for safety
        out << "    " << instr << " " << size_prefix << " [" << base_reg << " + " << offset << "], " << src_val << std::endl;
    } else {
        std::string final_src = src_val;
        // Handle register sizing for the source
        if (src_val == "rbx" || src_val == "rax") {
            if (size == 1) final_src = (src_val == "rbx") ? "bl" : "al";
            else if (size == 4) final_src = (src_val == "rbx") ? "ebx" : "eax";
            else final_src = src_val; // Keep rbx/rax for 8-byte
        }

        out << "    mov " << size_prefix << " [" << base_reg << " + " << offset << "], " 
            << final_src << std::endl;
    }
}

void CodeGenerator::emit_adv(const std::unique_ptr<TypeNode>& type, const std::string& base_reg,
                             int offset, const std::string& src_val) {
    int size = getTypeSize(type.get());
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_double = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT);
    std::string size_prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";

    if (is_float || is_double) {
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

void CodeGenerator::emit_mov_global(const std::string& reg, const std::string& label, int size) {
    std::string prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";
    emit("mov", reg, prefix + " [rel " + label + "]");
}

void CodeGenerator::call_external(const std::string& func_name) {
    bool misaligned = (current_stack_depth % 16 != 0);

    if (misaligned) {
        emit("sub", "rsp", "8");
    }

    emit("call", func_name);

    if (misaligned) {
        emit("add", "rsp", "8");
    }
}

void CodeGenerator::emit_print(const std::shared_ptr<TypeNode>& type) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    int size = getTypeSize(type.get());

    if (isFloatingPoint(type)) {
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

void CodeGenerator::emit_binary_op(const std::string& op_instr, char type) {
    if (type == 'f' || type == 'l') {
        std::string suffix = (type == 'f') ? "ss" : "sd";
        std::string fp_op = op_instr;
        if (op_instr == "imul") {
            fp_op = "mul";  // imul -> vmulss
        }
        if (op_instr == "idiv") {
            fp_op = "div";  // idiv -> vdivss
        }

        out << "    v" << fp_op << suffix << " xmm0, xmm1, xmm0" << std::endl;
    } else {
        if (op_instr == "idiv") {
            emit("mov", "rcx", "rax");
            emit("mov", "rax", "rbx");
            emit("cqo");
            out << "    idiv rcx" << std::endl;
        } else {
            emit(op_instr, "rbx", "rax");
            emit("mov", "rax", "rbx");
        }
    }
}

void CodeGenerator::load_adv(const std::shared_ptr<TypeNode>& type, const std::string& dest_reg,
                             const std::string base_reg, int offset) {
    bool is_fp = isFloatingPoint(type);
    int size = getTypeSize(type.get());
    std::string off_str = std::to_string(offset);
    std::string size_prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";

    if (is_fp) {
        std::string instr = (size == 4) ? "vmovss" : "vmovsd";
        emit(instr, dest_reg, size_prefix + " [" + base_reg + " + " + off_str + "]");
    } else {
        if (size == 1 || size == 4) {
            emit("movsx", dest_reg, size_prefix + " [" + base_reg + " + " + off_str + "]");
        } else {
            emit("mov", dest_reg, "qword [" + base_reg + " + " + off_str + "]");
        }
    }
}

void CodeGenerator::load_adv(const std::unique_ptr<TypeNode>& type, const std::string& dest_reg,
                             const std::string& base_reg, int offset) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_fp = (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT || prim->primitive_type == Token::KEYWORD_DOUBLE);
    int size = getTypeSize(type.get());
    std::string off_str = std::to_string(offset);
    std::string size_prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";

    if (is_fp) {
        std::string instr = (size == 4) ? "vmovss" : "vmovsd";
        emit(instr, dest_reg, size_prefix + " [" + base_reg + " + " + off_str + "]");
    } else {
        if (size == 1 || size == 4) {
            emit("movsx", dest_reg, size_prefix + " [" + base_reg + " + " + off_str + "]");
        } else {
            emit("mov", dest_reg, "qword [" + base_reg + " + " + off_str + "]");
        }
    }
}

bool CodeGenerator::isFloatingPoint(const std::shared_ptr<TypeNode>& type) {
    if (!type) {
        return false;
    }
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    return (prim != nullptr) && (prim->primitive_type == Token::KEYWORD_FLOAT ||
                                 prim->primitive_type == Token::KEYWORD_DOUBLE);
}
