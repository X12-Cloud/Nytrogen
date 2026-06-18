#ifndef INSTRUCTION_SET_HPP
#define INSTRUCTION_SET_HPP

#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

#include "ast.hpp"
#include "symbol_table.hpp"
#include "register_allocator.hpp"
#include "utils.hpp"

class InstructionSet {
    std::ofstream& out;
    int current_stack_depth = 0;
public:
    InstructionSet(std::ofstream& o) : out(o) {}

    // Instruction set
    std::string get_size_prefix(int size);
    int get_stack_depth() const { return current_stack_depth; }
    void reset_stack_depth() { current_stack_depth = 0; }
    bool isAFloatingPoint(const TypeNode* type);

    void emit(const std::string& instr);
    void emit(const std::string& instr, const std::string reg);
    void emit(const std::string& instr, const std::string& dest, const std::string& src);
    void push(const std::string& reg);
    void pop(const std::string& reg);
    void call_external(const std::string& func_name);

    void emit_adv(int size, const TypeNode* type, const std::string& base_reg, int offset,
                  const std::string& src_val);
    void load_adv(int size, const TypeNode* type, const std::string& dest_reg,
                  const std::string base_reg, int offset);
    void emit_print(int size, const std::shared_ptr<TypeNode>& type);

    void emit_binary_op(const std::string& op_instr, char type);
};

#endif  // INSTRUCTION_SET_HPP
