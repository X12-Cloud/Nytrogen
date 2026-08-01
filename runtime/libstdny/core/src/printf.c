#include <stdio.h>
#include <stdlib.h>

void ny_print_int(long long val) { printf("%lld\n", val); }
void ny_print_int_raw(long long val) { printf("%lld", val); }

void ny_print_float(double val) { printf("%f\n", val); }
void ny_print_float_raw(double val) { printf("%f", val); }

void ny_print_string(const char* val) { printf("%s\n", val); }
void ny_print_string_raw(const char* val) { printf("%s", val); }

void ny_print_char(char val) { printf("%c\n", val); }
void ny_print_char_raw(char val) { printf("%c", val); }

void ny_print_bool(int val) { printf(val ? "true\n" : "false\n"); }
void ny_print_bool_raw(int val) { printf(val ? "true" : "false"); }

void ny_print_complex(double re, double im) { printf("(%.4f + %.4fi)\n", re, im); }
