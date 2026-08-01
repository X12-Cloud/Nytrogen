#include <cstdio>

extern "C" double _N_qlib_state[]; 

extern "C" void print_q(size_t offset) {
    int q_idx = (int)(offset / 32);
    size_t s = offset / 8;

    printf("--- Q%d (Raw Amplitudes) ---\n", q_idx);
    printf("  |0>: %.4f + %.4fi\n", _N_qlib_state[s],   _N_qlib_state[s+1]);
    printf("  |1>: %.4f + %.4fi\n", _N_qlib_state[s+2], _N_qlib_state[s+3]);
    printf("---------------------------\n");
}
