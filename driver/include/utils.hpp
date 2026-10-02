#ifndef DRIVER_UTILS_HPP
#define DRIVER_UTILS_HPP

#include <iostream>
#include <vector>
#include <fstream>
#include <unordered_map>

inline void clean_q_file(const std::string& filepath) {
    std::ifstream input(filepath);
    if (!input.is_open()) return;

    std::unordered_map<int, std::string> final_states;
    std::vector<int> index_order;

    std::string line;
    while (std::getline(input, line)) {
        if (line.empty()) continue;

        size_t colon_pos = line.find(':');
        if (colon_pos != std::string::npos) {
            try {
                int qubit_idx = std::stoi(line.substr(0, colon_pos));

                if (final_states.find(qubit_idx) == final_states.end()) {
                    index_order.push_back(qubit_idx);
                }

                final_states[qubit_idx] = line;
            } catch (...) {
                // Ignore other lines
            }
        }
    }
    input.close();

    std::ofstream output(filepath, std::ios::trunc);
    for (int idx : index_order) {
        output << final_states[idx] << "\n";
    }
}

#endif
