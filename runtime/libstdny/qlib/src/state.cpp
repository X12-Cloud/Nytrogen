#include <cstdio>
#include <cstdint>
#include <cstring>
#include <cstdlib>

extern "C" double _N_qlib_state[];
static char result_buffer[256];

struct qubit_rep {
    int idx;
    double a_r, a_i, b_r, b_i;
    double x, y, z;
};

extern "C" {

qubit_rep fetch_data(size_t offset) {
    int q_idx = static_cast<int>(offset / 32);
    size_t s = offset / 8;

    double a_r = _N_qlib_state[s];
    double a_i = _N_qlib_state[s+1];
    double b_r = _N_qlib_state[s+2];
    double b_i = _N_qlib_state[s+3];

    // calculate bloch sphere coordinates
    // x = 2 * Re(a * b*)
    // y = 2 * Im(a* * b)
    // z = |a|^2 - |b|^2
    double x = 2.0 * (a_r * b_r + a_i * b_i);
    double y = 2.0 * (a_r * b_i - a_i * b_r);
    double z = (a_r * a_r + a_i * a_i) - (b_r * b_r + b_i * b_i);

    return { q_idx, a_r, a_i, b_r, b_i, x, y, z };
}

const char* qubit_to_str(size_t offset) {
    qubit_rep q = fetch_data(offset);

    snprintf(result_buffer, sizeof(result_buffer),
             "%d:a%+.6f%+.6fi|b%+.6f%+.6fi [x:%.4f, y:%.4f, z:%.4f]",
             q.idx, q.a_r, q.a_i, q.b_r, q.b_i, q.x, q.y, q.z);

    return result_buffer;
}

void print_q_file(size_t offset) {
    qubit_rep q = fetch_data(offset);

    FILE* f = fopen("/tmp/qlib_state.txt", "a");
    if (f) {
        fprintf(f, "%d:a%+.6f%+.6fi|b%+.6f%+.6fi [x:%.4f, y:%.4f, z:%.4f]\n",
                q.idx, q.a_r, q.a_i, q.b_r, q.b_i, q.x, q.y, q.z);
        fclose(f);
    }
}

char* dedupe_q_str(const char* input) {
    if (!input) return NULL;

    size_t len = strlen(input);
    char* result = (char*)malloc(len + 1);
    if (!result) return NULL;

    const char* read = input;
    char* write = result;

    while (*read) {
        const char *line_start = read;
        const char *line_end = strchr(read, '\n');
        size_t line_len = line_end ? (size_t)(line_end - line_start) : strlen(line_start);
        read = line_end ? line_end + 1 : line_start + line_len;

        int idx = 0;
        int is_q_line = (sscanf(line_start, "%d:", &idx) == 1);

        if (is_q_line) {
            const char *future = read;
            int has_later_duplicate = 0;

            while (*future) {
                int future_idx = -1;
                if (sscanf(future, "%d:", &future_idx) == 1 && future_idx == idx) {
                    has_later_duplicate = 1;
                    break;
                }
                const char *next_nl = strchr(future, '\n');
                if (!next_nl) break;
                future = next_nl + 1;
            }

            if (has_later_duplicate) continue;
        }

        memcpy(write, line_start, line_len);
        write += line_len;
        if (line_end) *write++ = '\n';
    }

    *write = '\0';
    return result;
}

} // extern "C"
