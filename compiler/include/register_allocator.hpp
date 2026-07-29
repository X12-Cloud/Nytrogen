#ifndef REGISTER_ALLOCATOR_HPP
#define REGISTER_ALLOCATOR_HPP

#include <regex>
#include <string>
#include <unordered_map>
#include <vector>

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

    struct LiveInterval {
        int end;
    };
    std::unordered_map<std::string, LiveInterval> intervals;

    void calculate_liveness(const std::vector<InstructionSet::Instruction>& instrs);
    auto needs_xmm(const std::string& mnemonic) -> bool;
    auto is_vreg(const std::string& op) -> bool;

    auto get_sized_reg(const std::string& phys, const std::string& current_op,
                       const InstructionSet::Instruction& instr) -> std::string;
};

#endif
