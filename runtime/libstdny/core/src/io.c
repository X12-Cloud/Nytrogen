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

int putst(const char* x) { puts(x); return 0; }
int putch(char c) { putchar(c); return 0; }

char* readln() {
    static char buffer[1024];
    if (fgets(buffer, 1024, stdin)) {
        buffer[strcspn(buffer, "\n")] = 0;
        return buffer;
    }
    return "";
}

char getch() { return (char)getchar(); }

// Memory management
void* ny_malloc(int size) {
    return malloc(size);
}

void ny_free(void* ptr) {
    free(ptr);
}
