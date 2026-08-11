#include <stdio.h>
#include <string.h>
#include <stdbool.h>

// Main types
void ny_print_int(FILE* stream, long long val, bool raw) {
    fprintf(stream, raw ? "%lld" : "%lld\n", val);
}

void ny_print_string(FILE* stream, const char* val, bool raw) {
    fprintf(stream, raw ? "%s" : "%s\n", val);
}

void ny_print_char(FILE* stream, char val, bool raw) {
    fprintf(stream, raw ? "%c" : "%c\n", val);
}

void ny_print_bool(FILE* stream, int val, bool raw) {
    const char* s = val ? "true" : "false";
    fprintf(stream, raw ? "%s" : "%s\n", s);
}

// Extra types
void ny_print_float(FILE* stream, double val) { printf("%f\n", val); }
void ny_print_float_raw(FILE* stream, double val) { printf("%f", val); }

void ny_print_complex(FILE* stream, double re, double im) { printf("(%.4f + %.4fi)\n", re, im); }

// Dispatcher
void ny_print(FILE* stream, void* val, const char* type, bool is_raw) {
    // Safety check for null stream
    if (!stream) stream = stdout;

    if (strcmp(type, "int") == 0) {
        ny_print_int(stream, (long long)val, is_raw);
    } else if (strcmp(type, "string") == 0) {
        ny_print_string(stream, (const char*)val, is_raw);
    } else if (strcmp(type, "char") == 0) {
        ny_print_char(stream, (char)(long long)val, is_raw);
    } else if (strcmp(type, "bool") == 0) {
        ny_print_bool(stream, (int)(long long)val, is_raw);
    } else if (strcmp(type, "float") == 0) {
        union { long long l; double d; } u;
        u.l = (long long)val;
        is_raw ? ny_print_float_raw(stream, u.d) : ny_print_float(stream, u.d);
    }
}
