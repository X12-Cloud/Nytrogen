#ifndef REGISTER_ALLOCATOR_HPP
#define REGISTER_ALLOCATOR_HPP

#include <vector>
#include <unordered_map>
#include <string>

struct RegStrings {
    std::string byte1;
    std::string byte4;
    std::string byte8;
};

class RegisterAllocator {
public:
    enum RegID {
        R10,
        R11,
        RBX,
        RCX,
        RDI,
        RSI
    };

    RegisterAllocator();

    RegID allocate();
    void free_reg(RegID reg);

    void reset();

    std::string get_name(RegID reg, int byte_size);

private:
    std::vector<RegID> register_pool;
    std::unordered_map<RegID, RegStrings> register_lookup;
};

#endif
