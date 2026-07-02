
section .text
    extern printf
    extern strcmp
    global calculate_score
    global main
_N15calculate_score:
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, 64
    mov [rbp + -8], rdi
    mov [rbp + -16], rsi
    movsx rax, dword [rbp + -8]
    push rax
    movsx rax, dword [rbp + -16]
    pop rbx
    add rcx, rbx
    cmp rcx, rax
    setg al
    movzx rax, al
    cmp rax, 0
    je _if_false_0
    movsx rax, dword [rbp + -8]
    push rax
    mov rax, 2
    pop rbx
    add rcx, rbx
    imul rbx, rax
    mov rax, rbx
    push rax
    movsx rax, dword [rbp + -16]
    push rax
    mov rax, 2
    pop rbx
    add rcx, rbx
    mov rcx, rax
    mov rax, rbx
    cqo
    idiv rcx
    pop rbx
    add rcx, rbx
    add rbx, rax
    mov rax, rbx
    jmp _N15calculate_score_epilogue
    jmp _if_end_0
_if_false_0:
_if_end_0:
    movsx rax, dword [rbp + -8]
    push rax
    movsx rax, dword [rbp + -16]
    pop rbx
    add rcx, rbx
    sub rbx, rax
    mov rax, rbx
    jmp _N15calculate_score_epilogue
