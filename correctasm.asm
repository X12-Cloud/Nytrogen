section .text
extern printf
extern strcmp
global _start
global calculate_score
global main
_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall
calculate_score:
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
    mov rcx, rbx
    cmp rcx, rax
    setg al
    movzx rax, al
    cmp rax, 0
    je _if_false_0
_if_true_0:
    movsx rax, dword [rbp + -8]
    push rax
    mov rax, 2
    pop rbx
    mov rcx, rbx
    imul rbx, rax
    mov rax, rbx
    push rax
    movsx rax, dword [rbp + -16]
    push rax
    mov rax, 2
    pop rbx
    mov rcx, rbx
    mov rcx, rax
    mov rax, rbx
    cqo
    idiv rcx
    pop rbx
    mov rcx, rbx
    add rbx, rax
    mov rax, rbx
    jmp calculate_score_epilogue
    jmp _if_end_0
_if_false_0:
_if_end_0:
    movsx rax, dword [rbp + -8]
    push rax
    movsx rax, dword [rbp + -16]
    pop rbx
    mov rcx, rbx
    sub rbx, rax
    mov rax, rbx
    jmp calculate_score_epilogue
calculate_score_epilogue:
    leave
    ret
main:
    push rbp
    mov rbp, rsp
    and rsp, -16
    sub rsp, 64
    lea rax, [rel _str_0]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, 2
    push rax
    mov rax, 0
    mov rbx, rax
    lea rax, [rbp + -12]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    mov rax, 0
    push rax
    mov rax, 1
    mov rbx, rax
    lea rax, [rbp + -12]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    mov rax, 1
    push rax
    mov rax, 2
    mov rbx, rax
    lea rax, [rbp + -12]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    mov rax, 100
    push rax
    mov rax, 0
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    mov rax, 200
    push rax
    mov rax, 1
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    mov rax, 300
    push rax
    mov rax, 2
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    lea rax, [rel _str_1]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, 0
    mov rbx, rax
    lea rax, [rbp + -12]
    imul rbx, 4
    add rax, rbx
    movsx rax, dword [rax]
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    movsx rax, dword [rax]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, 999
    push rax
    mov rax, 1
    mov rbx, rax
    lea rax, [rbp + -12]
    imul rbx, 4
    add rax, rbx
    movsx rax, dword [rax]
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    pop rbx
    mov dword [rax + 0], ebx
    lea rax, [rel _str_2]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, 0
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    movsx rax, dword [rax]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, 10
    push rax
    lea rax, [rel _N1p]
    pop rbx
    mov dword [rax + 0], ebx
    mov rax, 20
    push rax
    lea rax, [rel _N1p]
    add rax, 4
    pop rbx
    mov dword [rax + 0], ebx
_while_start_0:
    mov rax, [rel _N1i]
    push rax
    mov rax, 3
    pop rbx
    mov rcx, rbx
    cmp rcx, rax
    setl al
    movzx rax, al
    cmp rax, 0
    je _while_end_0
    lea rax, [rel _str_3]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, [rel _N5total]
    push rax
    mov rax, [rel _N1i]
    mov rbx, rax
    lea rax, [rbp + -24]
    imul rbx, 4
    add rax, rbx
    movsx rax, dword [rax]
    pop rbx
    mov rcx, rbx
    add rbx, rax
    mov rax, rbx
    mov [rel _N5total], rax
    mov rax, [rel _N1i]
    push rax
    mov rax, 1
    pop rbx
    mov rcx, rbx
    add rbx, rax
    mov rax, rbx
    mov [rel _N1i], rax
    jmp _while_start_0
_while_end_0:
    lea rax, [rel _str_3]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    lea rax, [rel _str_4]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    vmovss xmm0, [rel _N4area]
    cvtss2sd xmm0, xmm0
    lea rdi, [rel _print_float_format]
    mov rax, 1
    ;sub rsp, 8
    call printf
    ;add rsp, 8
    lea rax, [rel _str_5]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, [rel _N12final_result]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    mov rax, [rel _N14current_status]
    push rax
    mov rax, 1
    pop rbx
    mov rcx, rbx
    cmp rcx, rax
    sete al
    movzx rax, al
    cmp rax, 0
    je _if_false_1
_if_true_1:
    mov rax, [rel _N12final_result]
    push rax
    mov rax, 1000
    pop rbx
    mov rcx, rbx
    cmp rcx, rax
    setg al
    movzx rax, al
    cmp rax, 0
    je _if_false_2
_if_true_2:
    lea rax, [rel _str_6]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
    jmp _if_end_2
_if_false_2:
    lea rax, [rel _str_7]
    mov rsi, rax
    lea rdi, [rel _print_str_format]
    xor rax, rax
    sub rsp, 8
    call printf
    add rsp, 8
_if_end_2:
    jmp _if_end_1
_if_false_1:
_if_end_1:
    mov rax, 0
    jmp main_epilogue
main_epilogue:
    leave
    ret

section .data
    _print_int_format db "%d", 10, 0
    _print_str_format db "%s", 10, 0
    _print_char_format db "%c", 10, 0
    _print_float_format db "%f", 10, 0
    _str_0 db "Hello", 0
    _N3map dq 0
    _N4data dq 0
    _str_1 db "nested index access (300):", 0
    _str_2 db "nested assignment result (999):", 0
    _N1p dq 0
    _N14current_status dq 0
    _N5total dq 0
    _N1i dq 0
    _str_3 db "processing index...", 0
    _N6radius dq 5.000000
    _N4area dd 0
    _str_4 db "circle area:", 0
    _N12final_result dq 0
    _str_5 db "final score:", 0
    _str_6 db "status: pass", 0
    _str_7 db "status: fail", 0
