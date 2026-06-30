#include "register_allocator.hpp"

#include <algorithm>

#include "utils/utils.hpp"

RegisterAllocator::RegisterAllocator() {
    register_pool = {R10, R11, RBX, RCX, RDI, RSI};

    register_lookup[R10] = {"r10b", "r10d", "r10"};
    register_lookup[R11] = {"r11b", "r11d", "r11"};
    register_lookup[RBX] = {"bl", "ebx", "rbx"};
    register_lookup[RCX] = {"cl", "ecx", "rcx"};
    register_lookup[RDI] = {"di", "edi", "rdi"};
    register_lookup[RSI] = {"si", "esi", "rsi"};
}

std::string RegisterAllocator::get_name(RegID reg, int byte_size) {
    const RegStrings& bundle = register_lookup[reg];

    switch (byte_size) {
        case 1:
            return bundle.byte1;
        case 4:
            return bundle.byte4;
        default:
            return bundle.byte8;
    }
}

RegisterAllocator::RegID RegisterAllocator::allocate() {
    if (register_pool.empty()) {
        Logger::report_error("Register Allocator", "Register pool is empty.");
    }

    RegID allocated_register = register_pool.back();
    register_pool.pop_back();

    return allocated_register;
}

void RegisterAllocator::free_reg(RegID reg) {
    auto it = std::find(register_pool.begin(), register_pool.end(), reg);

    if (it == register_pool.end()) {
        register_pool.push_back(reg);
    }
}

void RegisterAllocator::reset() {
    register_pool = {R10, R11, RBX, RCX, RDI, RSI};
    return;
}
