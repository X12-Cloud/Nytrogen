#include <sys/io.ny>

int main() {
    char buffer[64];

    // Read from stdin (File Descriptor 0)
    int bytes = sys_read(0, buffer, 64);

    // Write back to stdout (File Descriptor 1)
    if (bytes > 0) {
        sys_write(1, buffer, bytes);
    }

    return 0;
}
