#include "code_generator.hpp"
#include <stdexcept>
#include <sstream>

void CodeGenerator::emit(const std::string& instr) {
    out << "    " << instr << std::endl;
}

void CodeGenerator::emit(const std::string& instr, const std::string reg) {
    out << "    " << instr << reg << std::endl;
}

void CodeGenerator::emit(const std::string& instr, const std::string& dest, const std::string& src) {
    out << "    " << instr << " " << dest << ", " << src << std::endl;
}

void CodeGenerator::emit_adv(const std::shared_ptr<TypeNode>& type, const std::string& base_reg, int offset, const std::string& src_val) {
    int size = getTypeSize(type.get());
    bool is_fp = isFloatingPoint(type);
    std::string size_prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";

    if (is_fp) {
        std::string instr = (size == 4) ? "vmovss" : "vmovsd";
        out << "    " << instr << " [" << base_reg << " + " << offset << "], " << src_val << std::endl;
    } else {
	std::string final_src = src_val;

        if (src_val == "rbx") {
            if (size == 1) final_src = "bl";
            else if (size == 4) final_src = "ebx";
        } else if (src_val == "rax") {
            if (size == 1) final_src = "al";
            else if (size == 4) final_src = "eax";
        }

        out << "    mov " << size_prefix << " [" << base_reg << " + " << offset << "], " << final_src << std::endl;
    }
}

void CodeGenerator::emit_adv(const std::unique_ptr<TypeNode>& type, const std::string& base_reg, int offset, const std::string& src_val) {
    int size = getTypeSize(type.get());
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_double = prim && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = prim && (prim->primitive_type == Token::KEYWORD_FLOAT);
    std::string size_prefix = (size == 1) ? "byte" : (size == 4) ? "dword" : "qword";

    if (is_float || is_double) {
        std::string instr = (size == 4) ? "vmovss" : "vmovsd";
        out << "    " << instr << " [" << base_reg << " + " << offset << "], " << src_val << std::endl;
    } else {
	std::string final_src = src_val;

        if (src_val == "rbx") {
            if (size == 1) final_src = "bl";
            else if (size == 4) final_src = "ebx";
        } else if (src_val == "rax") {
            if (size == 1) final_src = "al";
            else if (size == 4) final_src = "eax";
        }

        out << "    mov " << size_prefix << " [" << base_reg << " + " << offset << "], " << final_src << std::endl;
    }
}

void CodeGenerator::emit_print(const std::shared_ptr<TypeNode>& type) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    int size = getTypeSize(type.get());

    if (isFloatingPoint(type)) {
        if (size == 4) emit("cvtss2sd", "xmm0", "xmm0");
        emit("lea", "rdi", "[rel _print_float_format]");
        emit("mov", "rax", "1");
    } else if (prim && prim->primitive_type == Token::KEYWORD_STRING) {
        emit("mov", "rsi", "rax");
        emit("lea", "rdi", "[rel _print_str_format]");
        emit("xor", "rax", "rax");
    } else if (prim && prim->primitive_type == Token::KEYWORD_CHAR) {
        emit("mov", "rsi", "rax");
        emit("lea", "rdi", "[rel _print_char_format]");
        emit("xor", "rax", "rax");
    } else {
        emit("mov", "rsi", "rax");
        emit("lea", "rdi", "[rel _print_int_format]");
        emit("xor", "rax", "rax");
    }
    emit("call", "printf");
}

void CodeGenerator::emit_binary_op(const std::string& op_instr, char type) {
    if (type == 'f' || type == 'l') {
	std::string suffix = (type == 'f') ? "ss" : "sd";
	std::string fp_op = op_instr;
        if (op_instr == "imul") fp_op = "mul"; // imul -> vmulss
        if (op_instr == "idiv") fp_op = "div"; // idiv -> vdivss

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

void CodeGenerator::load_adv(const std::shared_ptr<TypeNode>& type, const std::string& dest_reg, const std::string base_reg, int offset) {
    bool is_fp = isFloatingPoint(type);
    int size = getTypeSize(type.get());
    std::string off_str = std::to_string(offset);

    if (is_fp) {
	if (size == 4) emit("vmovss", dest_reg, "[" + base_reg + " + " + off_str + "]");
	else emit("vmovsd", dest_reg, "[" + base_reg + "+" + off_str + "]");
    } else {
	if (size == 1) emit("movsx", dest_reg, "byte [" + base_reg + " + " + off_str + "]");
	else if (size == 4) emit("movsx", dest_reg, "dword [" + base_reg + " + " + off_str + "]");
	else emit("mov", dest_reg, "[" + base_reg + " + " + off_str + "]");
    }
}

void CodeGenerator::load_adv(const std::unique_ptr<TypeNode>& type, const std::string& dest_reg, const std::string& base_reg, int offset) {
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    bool is_double = prim && (prim->primitive_type == Token::KEYWORD_DOUBLE);
    bool is_float = prim && (prim->primitive_type == Token::KEYWORD_FLOAT);
    int size = getTypeSize(type.get());
    std::string off_str = std::to_string(offset);

    if (is_float || is_double) {
	if (size == 4) emit("vmovss", dest_reg, "[" + base_reg + " + " + off_str + "]");
	else emit("vmovsd", dest_reg, "[" + base_reg + "+" + off_str + "]");
    } else {
	if (size == 1) emit("movsx", dest_reg, "byte [" + base_reg + " + " + off_str + "]");
	else if (size == 4) emit("movsx", dest_reg, "dword [" + base_reg + " + " + off_str + "]");
	else emit("mov", dest_reg, "[" + base_reg + " + " + off_str + "]");
    }
}

bool CodeGenerator::isFloatingPoint(const std::shared_ptr<TypeNode>& type) {
    if (!type) return false;
    auto prim = dynamic_cast<PrimitiveTypeNode*>(type.get());
    return prim && (prim->primitive_type == Token::KEYWORD_FLOAT || 
                    prim->primitive_type == Token::KEYWORD_DOUBLE);
}
