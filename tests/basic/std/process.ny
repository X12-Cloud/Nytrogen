#include <sys.nyt>

int main() {
    print "Parent PID: ", std::sys_getpid();
    int pid = std::sys_fork();
    if (pid == 0) {
        print "Child process active. My PID: ", std::sys_getpid();
        std::sys_yield(); // Give up time slice
        return 0;
    } else {
        print "Parent process spawned child with PID: ", pid;
        int status = 0;
        char* ptr = "x";
        std::sys_waitpid(pid, ptr, 0);
        print "Child process finished.";
    }
    return 0;
}
