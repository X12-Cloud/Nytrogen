#include <iostream>
#include <iomanip>
#include <string>

extern "C" void q_init(void* target);
extern "C" void q_h(size_t offset);
extern "C" void q_x(size_t offset);
extern "C" void setup(void* base);

// Let's give you a bigger playground (256 qubits)
const int MAX = 256;
alignas(32) double state_vector[MAX * 4] = {0}; 

void print_q(int q_idx) {
    if (q_idx < 0 || q_idx >= MAX) return;
    int s = q_idx * 4;
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "--- Q" << q_idx << " ---\n";
    std::cout << "A0: " << state_vector[s] << " + " << state_vector[s+1] << "i\n";
    std::cout << "A1: " << state_vector[s+2] << " + " << state_vector[s+3] << "i\n";
    std::cout << "----------\n";
}

// Helper to turn "q1" or "1" into the integer 1
int parse_idx(std::string s) {
    if (s[0] == 'q') return std::stoi(s.substr(1));
    return std::stoi(s);
}

int main() {
    setup(&state_vector[0]);
    std::string cmd, target;
    void* base = &state_vector[0];

    while (true) {
        std::cout << "qlib emu> ";
        if (!(std::cin >> cmd)) break;
        if (cmd == "exit") break;

        std::cin >> target;
        try {
            int idx = parse_idx(target);
            if (idx >= MAX) {
                std::cout << "Error: Max qubits is " << MAX << "\n";
                continue;
            }

            if (cmd == "init") {
                q_init(&state_vector[idx * 4]); 
                print_q(idx);
            } 
            else if (cmd == "h") {
		        size_t off = idx * 32;
		        asm volatile (
    		        "mov %0, %%rdi\n\t"
    		        "call q_h"
    		        : 
    		        : "r"(off) 
    		        : "rdi", "ymm0", "ymm1", "ymm2", "ymm3", "memory" 
		        );
                print_q(idx);
            } else if (cmd == "x") {
		        size_t off = idx * 32;
		        asm volatile (
    		        "mov %0, %%rdi\n\t"
    		        "call q_x"
    		        : 
    		        : "r"(off) 
    		        : "rdi", "ymm0", "ymm1", "ymm2", "ymm3", "memory" 
		        );
                print_q(idx);
            }
        } catch (...) { std::cout << "Invalid target\n"; }
    }
    return 0;
}
