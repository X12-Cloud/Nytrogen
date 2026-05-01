int sys_read(int fd, char* data, int count) {
    int result;
    asm {
        "mov rax, 0"
        "syscall"
        "mov [rbp - 8], eax"
    }
    return result;
}

int sys_write(int fd, char* data, int count) {
    int result;
    asm {
        "mov rax, 1"
        "syscall"
        "mov [rbp - 8], eax"
    }
    return result;
}

// File io
int sys_open(string path, int flags, int mode) {
    int fd;
    asm {
        "mov rax, 2"
        "mov rdi, rdi"
        "mov rsi, rsi"
        "mov rdx, rdx"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return fd;
}

int sys_close(int fd) {
    int result;
    asm {
        "mov rax, 3"
        "syscall"
        "mov [rbp - 8], eax"
    }
    return result;
}
