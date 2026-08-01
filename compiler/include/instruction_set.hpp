#ifndef INSTRUCTION_SET_HPP
#define INSTRUCTION_SET_HPP

#include <cmath>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>
#include <set>

#include "ast.hpp"

class InstructionSet {
    std::ofstream& out;
    int current_stack_depth = 0;
    std::set<std::string> needed_externs;

   public:
    InstructionSet(std::ofstream& o) : out(o) {}

    struct Instruction {
        std::string mnemonic;
        std::vector<std::string> operands;
    };
    std::vector<Instruction> instructions;

    void flush_to_file();

    // Helpers
    auto get_size_prefix(int size) -> std::string;
    [[nodiscard]] auto get_stack_depth() const -> int {
        return current_stack_depth;
    }
    void reset_stack_depth() {
        current_stack_depth = 0;
    }
    auto isAFloatingPoint(const TypeNode* type) -> bool;

    // Basic Emitters
    void emit_raw(const std::string& mnemonic, const std::vector<std::string>& operands) {
        instructions.push_back({mnemonic, operands});
    }
    void emit(const std::string& instr);
    void emit(const std::string& instr, std::string reg);
    void emit(const std::string& instr, const std::string& dest, const std::string& src);

    // Symbols & Labels
    void global(const std::string& name);
    void section(const std::string& section_name);
    void extern_sym(const std::string& name);
    void label(const std::string& label_name);
    void label_local(const std::string& name);

    // Stack & System
    void syscall(int code);
    void push(const std::string& reg);
    void pop(const std::string& reg);
    void call_external(const std::string& func_name);

    // Memory Access
    void mov_indirect(const std::string& base, int offset, const std::string& src);
    void emit_mem_rel(const std::string& instr, const std::string& label, const std::string& reg);
    void emit_lea_stack(const std::string& dest_vreg, int offset);
    void emit_load_constant(const std::string& instr, const std::string& dest_vreg,
                            const std::string& label);
    void emit_dereference(const std::string& dest_vreg, const std::string& src_vreg);
    void emit_data_entry(const std::string& label, const std::string& type,
                         const std::string& value);

    void emit_adv(int size, const TypeNode* type, const std::string& base_vreg, int offset,
                  const std::string& src_vreg);
    void load_adv(int size, const TypeNode* type, const std::string& dest_vreg,
                  const std::string& base_vreg, int offset);
    void load_from_address(int size, const TypeNode* type, const std::string& dest_vreg,
                           const std::string& addr_vreg);

    // Logical & Arithmetic
    void emit_binary_op(const std::string& op_instr, char type, const std::string& dest,
                        const std::string& left, const std::string& right);
    void emit_cmp(const std::string& op_instr, const std::string& dest_vreg,
                  const std::string& left_vreg, const std::string& right_vreg,
                  bool is_string_compare);

    // Printing
    void emit_print(int size, const std::shared_ptr<TypeNode>& type, const std::string& src_vreg);
    void emit_print_int(const std::string& src_vreg);
    void emit_print_raw(int size, const std::shared_ptr<TypeNode>& type,
                        const std::string& src_vreg);
    void emit_print_internal(int size, const std::shared_ptr<TypeNode>& type,
                                         const std::string& src_vreg, bool is_raw);
};

#endif
