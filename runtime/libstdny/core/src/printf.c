#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

// Main types
void ny_print_int(int fd, long long val, bool raw) {
    dprintf(fd, raw ? "%lld" : "%lld\n", val);
}

void ny_print_string(int fd, const char* val, bool raw) {
    dprintf(fd, raw ? "%s" : "%s\n", val);
}

void ny_print_char(int fd, char val, bool raw) {
    dprintf(fd, raw ? "%c" : "%c\n", val);
}

void ny_print_bool(int fd, int val, bool raw) {
    const char* s = val ? "true" : "false";
    dprintf(fd, raw ? "%s" : "%s\n", s);
}

// Extra types
void ny_print_float(int fd, double val) { dprintf(fd, "%f\n", val); }
void ny_print_float_raw(int fd, double val) { dprintf(fd, "%f", val); }

void ny_print_complex(int fd, double re, double im) { 
    dprintf(fd, "(%.4f + %.4fi)\n", re, im); 
}

void ny_print_matrix(const double* data, uint64_t rows, uint64_t cols) {
    if (!data) {
        printf("[\n  (null matrix)\n]\n");
        return;
    }

    printf("[\n");
    for (uint64_t i = 0; i < rows; ++i) {
        printf("  [ ");
        for (uint64_t j = 0; j < cols; ++j) {
            uint64_t base_idx = (i * cols + j) * 2;
            double real = data[base_idx];
            double imag = data[base_idx + 1];

            printf("[%.4f, %.4f]", real, imag);

            if (j + 1 < cols) {
                printf(", ");
            }
        }
        printf(" ]\n");
    }
    printf("]\n");
}

// Dispatcher
void ny_print(int fd, void* val, const char* type, bool is_raw) {
    if (strcmp(type, "int") == 0) {
        ny_print_int(fd, (long long)val, is_raw);
    } else if (strcmp(type, "string") == 0) {
        ny_print_string(fd, (const char*)val, is_raw);
    } else if (strcmp(type, "char") == 0) {
        ny_print_char(fd, (char)(long long)val, is_raw);
    } else if (strcmp(type, "bool") == 0) {
        ny_print_bool(fd, (int)(long long)val, is_raw);
    } else if (strcmp(type, "float") == 0) {
        union { long long l; double d; } u;
        u.l = (long long)val;
        is_raw ? ny_print_float_raw(fd, u.d) : ny_print_float(fd, u.d);
    }
}
