#ifndef INSTRUCTION_SET_HPP
#define INSTRUCTION_SET_HPP

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "register_allocator.hpp"
#include "symbol_table.hpp"
#include "utils/utils.hpp"

class InstructionSet {
    std::ofstream& out;
    std::ofstream debug_out;
    int current_stack_depth = 0;

   public:
    InstructionSet(std::ofstream& o) : out(o), debug_out("debug_asm.txt") {}

    struct Instruction {
        std::string mnemonic;
        std::vector<std::string> operands;
    };
    std::vector<Instruction> instructions;

    void flush_to_file() {
        for (size_t i = 0; i < instructions.size(); ++i) {
            const auto& instr = instructions[i];
            if (instr.mnemonic.empty()) continue;

            if (instr.mnemonic.back() == ':') {
                out << "\n" << instr.mnemonic;
                if (i + 1 < instructions.size() && 
                   (instructions[i+1].mnemonic == "db" || 
                    instructions[i+1].mnemonic == "dw" || 
                    instructions[i+1].mnemonic == "dd" || 
                    instructions[i+1].mnemonic == "dq")) {
                    out << " " << instructions[i+1].mnemonic;
                    for (const auto& op : instructions[i+1].operands) out << " " << op;
                    out << "\n";
                    i++;
                } else {
                    out << "\n";
                }
            } else if (instr.mnemonic == "section") {
                out << "\nsection " << instr.operands[0] << "\n";
            } else {
                out << "    " << instr.mnemonic;
                for (size_t i = 0; i < instr.operands.size(); ++i) {
                    out << (i == 0 ? " " : ", ") << instr.operands[i];
                }
                out << "\n";
            }
        }
        instructions.clear();
    }

    // Instruction set
    std::string get_size_prefix(int size);
    int get_stack_depth() const {
        return current_stack_depth;
    }
    void reset_stack_depth() {
        current_stack_depth = 0;
    }
    bool isAFloatingPoint(const TypeNode* type);

    void emit_raw(const std::string& mnemonic, const std::vector<std::string>& operands) {
        instructions.push_back({mnemonic, operands});
    }

    void emit(const std::string& instr);
    void emit(const std::string& instr, const std::string reg);
    void emit(const std::string& instr, const std::string& dest, const std::string& src);
    void global(const std::string& name);
    void section(const std::string& section_name);
    void extern_sym(const std::string& name);
    void label(const std::string& label);
    void label_local(const std::string& name);
    void mov_indirect(const std::string& base, int offset, const std::string& src);
    void write_raw(const std::string& data);
    void emit_mem_rel(const std::string& instr, const std::string& label, const std::string& reg);
    void emit_cmp(const std::string& op, bool is_string_compare);
    void syscall(int code);
    void push(const std::string& reg);
    void pop(const std::string& reg);
    void call_external(const std::string& func_name);
    void emit_lea_stack(int offset);
    void emit_load_constant(const std::string& instr, const std::string& label);
    void emit_dereference();
    void emit_data_entry(const std::string& label, const std::string& type,
                         const std::string& value);

    void emit_adv(int size, const TypeNode* type, const std::string& base_reg, int offset,
                  const std::string& src_val);
    void load_adv(int size, const TypeNode* type, const std::string& dest_reg,
                  const std::string base_reg, int offset);
    void load_from_address(int size, const std::string& reg);
    void emit_print(int size, const std::shared_ptr<TypeNode>& type);
    void emit_print_int(const std::string& reg);
    void emit_print_raw(int size, const std::shared_ptr<TypeNode>& type);

    void emit_binary_op(const std::string& op_instr, char type);
};

#endif  // INSTRUCTION_SET_HPP
