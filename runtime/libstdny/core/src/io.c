#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

// Standard io
int println(const char* x) {
    printf("%s\n", x);
    return 0;
}

// int putchar(int c)
// int puts(const char* s)

char* readln() {
    static char buffer[1024];
    if (fgets(buffer, 1024, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        return buffer;
    }
    return "";
}

// int getchar()

// Memory management
// void* malloc(size_t size)
// void free(void* ptr);
