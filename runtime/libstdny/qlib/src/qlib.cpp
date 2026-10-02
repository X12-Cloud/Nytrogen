#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>

extern "C" double _N_qlib_state[];
static char result_buffer[256];

extern "C" {

void q_init(void* target);
void print_q_file(size_t offset);
struct qubit_rep;

void print_q(size_t offset) {
    int q_idx = static_cast<int>(offset / 32);
    size_t s = offset / 8;

    printf("--- Q%d (Raw Amplitudes) ---\n", q_idx);
    printf("  |0>: %.4f + %.4fi\n", _N_qlib_state[s],   _N_qlib_state[s+1]);
    printf("  |1>: %.4f + %.4fi\n", _N_qlib_state[s+2], _N_qlib_state[s+3]);
    printf("---------------------------\n");
}

void qubit_init(void* target) {
    q_init(target);
    size_t offset = (uintptr_t)target - (uintptr_t)_N_qlib_state;

    print_q_file(offset);
}

} // extern "C"
