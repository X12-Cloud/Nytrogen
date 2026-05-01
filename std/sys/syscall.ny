int syscall0(int syscall_number) {
    int ret;
    asm {
        "mov rax, rdi"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}

int syscall1(int syscall_number, int arg1) {
    int ret;
    asm {
        "mov rax, rdi"
        "mov rdi, rsi"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}

int syscall2(int syscall_number, int arg1, int arg2) {
    int ret;
    asm {
        "mov rax, rdi"
        "mov rdi, rsi"
        "mov rsi, rdx"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}

int syscall3(int syscall_number, int arg1, int arg2, int arg3) {
    int ret;
    asm {
        "mov rax, rdi"
        "mov rdi, rsi"
        "mov rsi, rdx"
        "mov rdx, rcx"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}

int syscall4(int syscall_number, int arg1, int arg2, int arg3, int arg4) {
    int ret;
    asm {
        "mov rax, rdi"
        "mov rdi, rsi"
        "mov rsi, rdx"
        "mov rdx, rcx"
        "mov r10, r8"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}

int syscall5(int syscall_number, int arg1, int arg2, int arg3, int arg4, int arg5) {
    int ret;
    asm {
        "mov rax, rdi"
        "mov rdi, rsi"
        "mov rsi, rdx"
        "mov rdx, rcx"
        "mov r10, r8"
        "mov r8, r9"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}

int syscall6(int syscall_number, int arg1, int arg2, int arg3, int arg4, int arg5, int arg6) {
    int ret;
    // arg6 is on the stack at [rbp + 16]
    asm {
        "mov rax, rdi"
        "mov rdi, rsi"
        "mov rsi, rdx"
        "mov rdx, rcx"
        "mov r10, r8"
        "mov r8, r9"
        "mov r9, [rbp + 16]"
        "syscall"
        "mov [rbp - 8], rax"
    }
    return ret;
}