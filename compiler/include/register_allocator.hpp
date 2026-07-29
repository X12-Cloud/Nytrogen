#ifndef REGISTER_ALLOCATOR_HPP
#define REGISTER_ALLOCATOR_HPP

#include <string>
#include <unordered_map>
#include <vector>
#include <regex>
#include "instruction_set.hpp"

class RegisterAllocator {
   public:
    RegisterAllocator();
    void allocate_registers(std::vector<InstructionSet::Instruction>& instructions);

   private:
    std::vector<std::string> int_pool;
    std::vector<std::string> xmm_pool;

    std::unordered_map<std::string, std::string> vreg_to_phys;

    std::unordered_map<std::string, std::string> spill_slots;
    int next_spill_offset;

    struct LiveInterval { int end; };
    std::unordered_map<std::string, LiveInterval> intervals;

    void calculate_liveness(const std::vector<InstructionSet::Instruction>& instrs);
    bool needs_xmm(const std::string& mnemonic);
    bool is_vreg(const std::string& op);

    std::string get_sized_reg(const std::string& phys, const std::string& current_op, const InstructionSet::Instruction& instr);
};

#endif
