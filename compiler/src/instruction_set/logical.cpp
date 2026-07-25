#include "instruction_set.hpp"
#include <stdexcept>

void InstructionSet::emit_binary_op(const std::string& op_instr, char type, 
                                   const std::string& dest, 
                                   const std::string& left, 
                                   const std::string& right) {
    if (type == 'f' || type == 'l') {
        std::string suffix = (type == 'f') ? "ss" : "sd";
        std::string fp_op = op_instr;

        if (op_instr == "add") fp_op = "add";
        else if (op_instr == "sub") fp_op = "sub";
        else if (op_instr == "imul") fp_op = "mul";
        else if (op_instr == "idiv") fp_op = "div";

        emit_raw("v" + fp_op + suffix, {dest, left, right});
    } else {
        if (op_instr == "idiv") {
            emit("mov", "rax", left);
            emit("cqo");
            emit("idiv", right);
            emit("mov", dest, "rax");
        } else {
            emit("mov", dest, left);
            emit(op_instr, dest, right);
        }
    }
}

void InstructionSet::emit_cmp(const std::string& op_instr, 
                             const std::string& dest_vreg, 
                             const std::string& left_vreg, 
                             const std::string& right_vreg, 
                             bool is_string_compare) {
    if (is_string_compare) {
        emit("mov", "rdi", left_vreg);
        emit("mov", "rsi", right_vreg);
        call_external("strcmp");
        emit("test", "rax", "rax");
    } else {
        emit("cmp", left_vreg, right_vreg);
    }

    emit(op_instr, "al"); 
    emit("movzx", dest_vreg, "al"); 
}
