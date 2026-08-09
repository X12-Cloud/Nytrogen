#include <sys/io.ny>

int main() {
    string msg = "Nytrogen Syscall Test\n";
    int bytes_written = sys_write(1, msg, 22);

    if (bytes_written < 0) {
        return 1;
    }

    return 0;
}
