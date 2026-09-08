#include "register_allocator.hpp"

#include <algorithm>
#include <iostream>

RegisterAllocator::RegisterAllocator() : next_spill_offset(128) {}

bool RegisterAllocator::is_vreg(const std::string& op) {
    return op.size() > 1 && op[0] == 'v' && std::isdigit(op[1]);
}

bool RegisterAllocator::needs_xmm(const std::string& mnemonic) {
    if (mnemonic.find("lea") == 0 || mnemonic.find("movsx") == 0) return false;

    if (mnemonic.find("2si") != std::string::npos) return false;

    return (mnemonic.find("vmov") == 0 || mnemonic.find("vadd") == 0 || 
            mnemonic.find("vsub") == 0 || mnemonic.find("vmul") == 0 || 
            mnemonic.find("vdiv") == 0 || mnemonic.find("vcvt") == 0 || 
            mnemonic.find("vand") == 0 || mnemonic.find("sqrt") == 0 || 
            mnemonic.find("ucom") == 0 || mnemonic.find("vcmp") == 0);
}

std::string RegisterAllocator::get_sized_reg(const std::string& phys, const std::string& current_op, const InstructionSet::Instruction& instr) {
    if (phys.empty() || phys.find("xmm") == 0) return phys;

    static std::unordered_map<std::string, std::vector<std::string>> sub_regs = {
        {"rbx", {"bl",   "ebx",  "rbx"}}, {"r10", {"r10b", "r10d", "r10"}},
        {"r11", {"r11b", "r11d", "r11"}}, {"r12", {"r12b", "r12d", "r12"}},
        {"r13", {"r13b", "r13d", "r13"}}, {"r14", {"r14b", "r14d", "r14"}},
        {"r15", {"r15b", "r15d", "r15"}}, {"r8",  {"r8b",  "r8d",  "r8"}}, 
        {"r9",  {"r9b",  "r9d",  "r9"}}
    };

    if (current_op.find("[") != std::string::npos || instr.mnemonic == "lea") {
        return sub_regs.at(phys)[2]; 
    }

    std::string m = instr.mnemonic;
    if (m.find("movsx") == 0 || m.find("movsxd") == 0 || 
        m.find("lea") == 0 || m.find("vcvtt") == 0) {
        return sub_regs.at(phys)[2];
    }

    bool is_dword = false;
    bool is_byte = false;
    for (const auto& op : instr.operands) {
        if (op.find("dword") != std::string::npos) is_dword = true;
        if (op.find("byte") != std::string::npos) is_byte = true;
    }

    if (is_byte)  return sub_regs.at(phys)[0];
    if (is_dword) return sub_regs.at(phys)[1];

    return sub_regs.at(phys)[2];
}

void RegisterAllocator::calculate_liveness(const std::vector<InstructionSet::Instruction>& instrs) {
    intervals.clear();
    std::regex e("(v\\d+)");
    for (int i = 0; i < (int)instrs.size(); ++i) {
        for (const auto& op : instrs[i].operands) {
            std::smatch m;
            std::string s = op;
            while (std::regex_search(s, m, e)) {
                intervals[m.str(1)].end = i;
                s = m.suffix().str();
            }
        }
    }
}

void RegisterAllocator::allocate_registers(std::vector<InstructionSet::Instruction>& instructions) {
    calculate_liveness(instructions);
    vreg_to_phys.clear();
    spill_slots.clear();
    next_spill_offset = 128;

    int_pool = {"r14", "r13", "r12", "rbx", "r11", "r10", "r9", "r8"};
    xmm_pool = {"xmm7", "xmm6", "xmm5", "xmm4", "xmm3", "xmm2", "xmm1"};

    std::vector<InstructionSet::Instruction> physical_instrs;

    for (int i = 0; i < (int)instructions.size(); ++i) {
        auto& instr = instructions[i];

        if (instr.mnemonic == "call") {
            for (auto it = vreg_to_phys.begin(); it != vreg_to_phys.end(); ) {
                std::string vreg = it->first;
                std::string phys = it->second;

                std::string slot = "[rbp - " + std::to_string(next_spill_offset) + "]";
                next_spill_offset += 8;
                spill_slots[vreg] = slot;

                std::string spill_instr = (phys.find("xmm") == 0) ? "vmovupd" : "mov";
                physical_instrs.push_back({spill_instr, {slot, phys}});

                if (phys.find("xmm") == 0) xmm_pool.push_back(phys);
                else int_pool.push_back(phys);

                it = vreg_to_phys.erase(it);
            }
        }

        // Expire intervals
        for (auto it = vreg_to_phys.begin(); it != vreg_to_phys.end();) {
            if (intervals[it->first].end < i) {
                if (it->second.find("xmm") == 0)
                    xmm_pool.push_back(it->second);
                else
                    int_pool.push_back(it->second);
                it = vreg_to_phys.erase(it);
            } else
                it++;
        }

        // Process operands
        for (auto& op : instr.operands) {
            std::regex e("(v\\d+)");
            std::smatch m;
            std::string search_str = op;
            std::string result_op = op;

            while (std::regex_search(search_str, m, e)) {
                std::string vreg = m.str(1);

                // Reload from stack if spilled
                if (spill_slots.count(vreg) && vreg_to_phys.find(vreg) == vreg_to_phys.end()) {
                    auto& pool = needs_xmm(instr.mnemonic) ? xmm_pool : int_pool;
                    if (pool.empty()) {
                        // Evict a victim to make room
                        std::string victim_v = vreg_to_phys.begin()->first;
                        std::string victim_p = vreg_to_phys.begin()->second;
                        std::string slot = "[rbp - " + std::to_string(next_spill_offset) + "]";
                        next_spill_offset += 8;
                        spill_slots[victim_v] = slot;
                        physical_instrs.push_back({"mov", {slot, victim_p}});
                        vreg_to_phys.erase(victim_v);
                        pool.push_back(victim_p);
                    }
                    std::string reg = pool.back();
                    pool.pop_back();
                    vreg_to_phys[vreg] = reg;
                    physical_instrs.push_back(
                        {(reg.find("xmm") == 0 ? "vmovss" : "mov"), {reg, spill_slots[vreg]}});
                }

                // Initial allocation if not in register
                if (vreg_to_phys.find(vreg) == vreg_to_phys.end()) {
                    auto& pool = needs_xmm(instr.mnemonic) ? xmm_pool : int_pool;
                    if (pool.empty()) {
                        std::string victim_v = vreg_to_phys.begin()->first;
                        std::string victim_p = vreg_to_phys.begin()->second;
                        std::string slot = "[rbp - " + std::to_string(next_spill_offset) + "]";
                        next_spill_offset += 8;
                        spill_slots[victim_v] = slot;
                        physical_instrs.push_back(
                            {(victim_p.find("xmm") == 0 ? "vmovss" : "mov"), {slot, victim_p}});
                        vreg_to_phys.erase(victim_v);
                        pool.push_back(victim_p);
                    }
                    vreg_to_phys[vreg] = pool.back();
                    pool.pop_back();
                }

                std::string phys = vreg_to_phys.at(vreg);
                std::string sized_name = get_sized_reg(phys, op, instr);
                result_op = std::regex_replace(result_op, std::regex(vreg), sized_name);
                search_str = m.suffix().str();
            }
            op = result_op;
        }
        physical_instrs.push_back(instr);
    }
    instructions = physical_instrs;
}
