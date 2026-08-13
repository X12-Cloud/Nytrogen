#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

// Standard io
int println(const char* x) {
    printf("%s\n", x);
    return 0;
}

int put(const char* x) { puts(x); return 0; }

int sys_write(int fd, const char* buf, int n) {
    write(fd, buf, n);
    return 0;
}

int sys_read(int fd, char* buf, int n) {
    return read(fd, buf, n);
}

int sys_open(const char* filename, int oflag) {
    open(filename, oflag);
}

int sys_close(int fd) {
    close(fd);
}
