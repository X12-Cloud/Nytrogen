#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <time.h>
#include <sched.h>
#include <errno.h>

// Executes a shell command via /bin/sh
int exec_cmd(const char* cmd) {
    char* argv[4];
    argv[0] = "/bin/sh";
    argv[1] = "-c";
    argv[2] = (char*)cmd;
    argv[3] = NULL;
    return execv("/bin/sh", argv);
}

// Custom wrapper to fetch file sizes safely
int file_size(const char* path) {
    struct stat st;
    if (stat(path, &st) == 0)
        return st.st_size;
    return -1;
}

// Thread context yielding wrapper
int yield_cpu() {
    return sched_yield();
}

// Retrieves thread-local error state
int last_error() {
    return errno;
}

// High-resolution monotonic timestamp provider
long get_nanos() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (long)ts.tv_sec * 1000000000L + ts.tv_nsec;
}
