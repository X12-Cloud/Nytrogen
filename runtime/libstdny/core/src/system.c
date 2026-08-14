#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sched.h>
#include <errno.h>

// I/O
int sys_open(const char* filename, int oflag) {
    return open(filename, oflag);
}

int sys_close(int fd) {
    return close(fd);
}

int sys_read(int fd, char* buf, int n) {
    return read(fd, buf, n);
}

int sys_write(int fd, const char* buf, int n) {
    return write(fd, buf, n);
}

// Process Management
int sys_fork() {
    return fork();
}

int sys_execve(const char* filename, char* const argv[], char* const envp[]) {
    return execve(filename, argv, envp);
}

int sys_waitpid(int pid, int* status, int options) {
    return waitpid(pid, status, options);
}

int sys_getpid() {
    return getpid();
}

// Filesystem & Errors
int sys_fsize(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return st.st_size;
    return -1;
}

int sys_last_error() {
    return errno;
}

// Time
long sys_get_nanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}

// Memory
void* sys_mmap(void* addr, size_t len, int prot, int flags, int fd, off_t offset) {
    return mmap(addr, len, prot, flags, fd, offset);
}

// Networking
int sys_socket(int domain, int type, int protocol) {
    return socket(domain, type, protocol);
}

int sys_bind(int sockfd, const void *addr, int addrlen) {
    return bind(sockfd, (const struct sockaddr *)addr, addrlen);
}

// Misc
int sys_yield() {
    return sched_yield();
}
