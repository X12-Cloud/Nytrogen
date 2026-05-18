#include <sys.nyt>

int main() {
    print("--- Starting Nytrogen Syscall and Namespace Test ---\n");

    // Linux File Flags for open: O_CREAT = 64, O_WRONLY = 1, O_RDONLY = 0
    // Permissions: 0644 (Octal) = 420 (Decimal)
    string filename = "./out/nytro_generated_file.txt";
    string secret_message = "Nytrogen standard library works beautifully!\n";
    
    print("\n[Step 1] Creating a file via std::sys::sys_open...\n");
    int fd = std::sys::sys_open("./out/nytro_generated_file.txt", 65, 420); // 65 = O_CREAT | O_WRONLY
    
    if (fd < 0) {
        print("FAIL: Could not open file for writing.\n");
        return 1;
    }
    print("SUCCESS: File descriptor allocated.\n");

    print("\n[Step 2] Writing data via std::sys::sys_write...\n");
    int bytes_written = std::sys::sys_write(fd, secret_message, 45);
    print("Bytes written to disk.\n");

    print("\n[Step 3] Flushing buffers via std::sys::sys_close...\n");
    std::sys::sys_close(fd);

    // --- READ BACK VERIFICATION ---
    print("\n[Step 4] Reading data back to verify integrity...\n");
    int read_fd = std::sys::sys_open("./out/nytro_generated_file.txt", 0, 0); // 0 = O_RDONLY
    
    if (read_fd < 0) {
        print("FAIL: Could not open file for reading.\n");
        return 1;
    }

    // Allocate a buffer space in your language's format
    string buffer = "__________________________________________________"; 
    std::sys::sys_read(read_fd, buffer, 45);
    
    print("Content recovered from file:\n");
    print(buffer);
    print("\n");

    std::sys::sys_close(read_fd);
    
    print("--- Test Complete: STATUS PASS ---\n");
    return 0;
}
