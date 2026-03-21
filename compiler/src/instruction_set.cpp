#include "code_generator.hpp"
#include <stdexcept>
#include <sstream>

void CodeGenerator::emit(std::string instruction, std::string dest, std::string data) {
    out << "    " << instruction << " " << dest << ", " << data << std::endl;
}

void CodeGenerator::emit_binary_op(const std::string& op_instr, char type) {
    std::string fp_op = op_instr;
    if (type == 'f' || type == 'l') {
        if (op_instr == "imul") fp_op = "mul"; // imul -> vmulss
        if (op_instr == "idiv") fp_op = "div"; // idiv -> vdivss
    }

    switch(type) {
	case 'd': // int
            if (op_instr == "idiv") {
                out << "    mov rcx, rax" << std::endl;
                out << "    mov rax, rbx" << std::endl;
                out << "    cqo" << std::endl;
                out << "    idiv rcx" << std::endl;
            } else {
                out << "    " << op_instr << " rbx, rax" << std::endl;
                out << "    mov rax, rbx" << std::endl;
            }
            break;
        case 'f': // float
            out << "    v" << fp_op << "ss xmm0, xmm1, xmm0" << std::endl;
            break;
        case 'l': // double
            out << "    v" << fp_op << "sd xmm0, xmm1, xmm0" << std::endl;
            break;
    }
}
