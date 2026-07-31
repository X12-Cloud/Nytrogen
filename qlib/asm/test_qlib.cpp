#include <iostream>
#include <iomanip>
#include <string>
#include <cmath> // Added for cos/sin

extern "C" {
    void q_init(void* target);
    void q_h(size_t offset);
    void q_x(size_t offset);
    void q_z(size_t offset);
    void q_s(size_t offset);
    void q_t(size_t offset);
    void q_rz(size_t offset, double cos_val, double sin_val);
    void setup(void* base);
}

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

int parse_idx(std::string s) {
    if (s[0] == 'q') return std::stoi(s.substr(1));
    return std::stoi(s);
}

int main() {
    setup(&state_vector[0]);
    std::string cmd, target;

    std::cout << "QLib Virtual Machine Loaded (" << MAX << " Qubits)\n";
    std::cout << "Commands: init, h, x, z, s, t, rz <deg>, exit\n";

    while (true) {
        std::cout << "qlib emu> ";
        if (!(std::cin >> cmd)) break;
        if (cmd == "exit") break;

        std::cin >> target;
        try {
            int idx = parse_idx(target);
            size_t off = idx * 32;

            if (cmd == "init") {
                q_init(&state_vector[idx * 4]); 
            } else if (cmd == "h") {
                q_h(off);
            } else if (cmd == "x") {
                q_x(off);
            } else if (cmd == "z") {
                q_z(off);
            } else if (cmd == "s") {
                q_s(off);
            } else if (cmd == "t") {
                q_t(off);
            } else if (cmd == "rz") {
                double deg;
                if (std::cin >> deg) { // Ensure deg is actually read
                    double rad = deg * M_PI / 180.0;
                    q_rz(off, cos(rad), sin(rad));
                }
            } else {
                std::cout << "Unknown command\n";
                continue;
            }
            print_q(idx);
        } catch (...) { std::cout << "Invalid target\n"; }
    }
    return 0;
}
