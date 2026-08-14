#define _GNU_SOURCE
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char* ny_format(const char* template_str, const char* types, ...) {
    if (!template_str || !types) return strdup("");

    char* c_fmt = malloc(strlen(template_str) * 8 + 1);
    char* d = c_fmt;
    int arg_ptr = 0;

    for (const char* s = template_str; *s != '\0'; s++) {
        if (*s == '{' && *(s+1) == '}') {
            s++;
            char t = types[arg_ptr++];
            if (t == 's')      { *d++ = '%'; *d++ = 's'; }
            else if (t == 'i') { *d++ = '%'; *d++ = 'l'; *d++ = 'l'; *d++ = 'd'; }
            else if (t == 'f') { *d++ = '%'; *d++ = 'f'; }
            else if (t == 'c') { *d++ = '%'; *d++ = 'c'; }
            else if (t == 'b') { *d++ = '%'; *d++ = 'l'; *d++ = 'l'; *d++ = 'd'; } // 1 or 0
        } else {
            *d++ = *s;
        }
    }
    *d = '\0';

    va_list args;
    va_start(args, types);
    char* result = NULL;

    if (vasprintf(&result, c_fmt, args) == -1) {
        result = strdup("FORMAT_ERR");
    }

    va_end(args);
    free(c_fmt);
    return result;
}
