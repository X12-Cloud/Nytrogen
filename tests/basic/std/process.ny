#include <sys.nyt>

int main() {
    print "Parent PID: ", sys::getpid();
    int pid = sys::fork();
    if (pid == 0) {
        print "Child process active. My PID: ", sys::getpid();
        sys::yield_cpu();
        return 0;
    } else {
        print "Parent process spawned child with PID: ", pid;
        int status = 0;
        char* ptr = "x";
        sys::waitpid(pid, ptr, 0);
        print "Child process finished.";
    }
    return 0;
}
