#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

// Standard io
int println(const char* x) {
    printf("%s\n", x);
    return 0;
}

int put(const char* x) { puts(x); return 0; }
