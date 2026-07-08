#include <sstream>
#include <stdexcept>

#include "instruction_set.hpp"

void InstructionSet::emit_binary_op(const std::string& op_instr, char type) {
    if (type == 'f' || type == 'l') {
        std::string suffix = (type == 'f') ? "ss" : "sd";
        std::string fp_op = op_instr;
        if (op_instr == "imul") {
            fp_op = "mul";  // imul -> vmulss
        }
        if (op_instr == "idiv") {
            fp_op = "div";  // idiv -> vdivss
        }

        emit_raw("v" + fp_op + suffix, {"xmm0", "xmm1", "xmm0"});
    } else {
        if (op_instr == "idiv") {
            emit("mov", "rcx", "rax");
            emit("mov", "rax", "rbx");
            emit("cqo");
            emit_raw("idiv", {"rcx"});
        } else {
            emit(op_instr, "rbx", "rax");
            emit("mov", "rax", "rbx");
        }
    }
}

void InstructionSet::emit_cmp(const std::string& op, bool is_string_compare) {
    if (is_string_compare) {
        emit("mov", "rdi", "rcx");
        emit("mov", "rsi", "rax");
        call_external("strcmp");
        emit("test", "rax", "rax");
    } else {
        emit("cmp", "rcx", "rax");
    }

    emit(op, "al");
    emit("movzx", "rax", "al");
}
