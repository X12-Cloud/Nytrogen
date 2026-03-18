section .data
  _print_int_format db "%d", 10, 0
  _print_str_format db "%s", 10, 0
  _print_char_format db "%c", 10, 0
section .text
global _start
extern printf
extern strcmp
calculate_score:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov [rbp + -8], rdi
    mov [rbp + -16], rsi
    mov rax, 0
    push rax
    lea rax, [rbp + -20]
    pop rbx
    mov [rax], ebx
    movsx rax, dword [rbp + -8]
    push rax
    movsx rax, dword [rbp + -16]
    mov rbx, rax
    pop rcx
    cmp rcx, rax
    setg al
    movzx rax, al
    cmp rax, 0
    je _if_false_0
_if_true_0:
    movsx rax, dword [rbp + -16]
    push rax
    mov rax, 0
    mov rbx, rax
    pop rcx
    cmp rcx, rax
    setne al
    movzx rax, al
    cmp rax, 0
    je _if_false_1
_if_true_1:
    movsx rax, dword [rbp + -8]
    push rax
    mov rax, 2
    mov rbx, rax
    pop rcx
    imul rcx, rbx
    mov rax, rcx
    push rax
    movsx rax, dword [rbp + -16]
    push rax
    mov rax, 2
    mov rbx, rax
    pop rcx
    mov rbx, rax
    mov rax, rcx
    cqo
    idiv rbx
    mov rbx, rax
    pop rcx
    add rcx, rbx
    mov rax, rcx
    push rax
    lea rax, [rbp + -20]
    pop rbx
    mov [rax], ebx
    jmp _if_end_1
_if_false_1:
_if_end_1:
    jmp _if_end_0
_if_false_0:
    movsx rax, dword [rbp + -8]
    push rax
    movsx rax, dword [rbp + -16]
    mov rbx, rax
    pop rcx
    sub rcx, rbx
    mov rax, rcx
    push rax
    lea rax, [rbp + -20]
    pop rbx
    mov [rax], ebx
_if_end_0:
    movsx rax, dword [rbp + -20]
    leave
    ret

main:
    push rbp
    mov rbp, rsp
    sub rsp, 64
    mov rax, 0
    push rax
    lea rax, [rbp + -4]
    pop rbx
    mov [rax], ebx
    mov rax, 0
    push rax
    lea rax, [rbp + -8]
    pop rbx
    mov [rax], ebx
    mov rax, 10
    push rax
    lea rax, [rbp + -16]
    pop rbx
    mov [rax], ebx
    mov rax, 20
    push rax
    lea rax, [rbp + -16]
    add rax, 4
    pop rbx
    mov [rax], ebx
    lea rax, [rbp + -16]
    movsx rax, dword [rax]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    lea rax, [rbp + -16]
    add rax, 4
    movsx rax, dword [rax]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
_while_start_0:
    movsx rax, dword [rbp + -4]
    push rax
    mov rax, 5
    mov rbx, rax
    pop rcx
    cmp rcx, rax
    setl al
    movzx rax, al
    cmp rax, 0
    je _while_end_0
    movsx rax, dword [rbp + -8]
    push rax
    lea rax, [rbp + -16]
    movsx rax, dword [rax]
    mov rbx, rax
    pop rcx
    add rcx, rbx
    mov rax, rcx
    push rax
    lea rax, [rbp + -8]
    pop rbx
    mov [rax], ebx
    movsx rax, dword [rbp + -4]
    push rax
    mov rax, 1
    mov rbx, rax
    pop rcx
    add rcx, rbx
    mov rax, rcx
    push rax
    lea rax, [rbp + -4]
    pop rbx
    mov [rax], ebx
    jmp _while_start_0
_while_end_0:
    mov rax, 1
    push rax
    lea rax, [rbp + -20]
    pop rbx
    mov [rax], ebx
    movsx rax, dword [rbp + -20]
    push rax
    mov rax, 1
    mov rbx, rax
    pop rcx
    cmp rcx, rax
    sete al
    movzx rax, al
    cmp rax, 0
    je _if_false_2
_if_true_2:
    movsx rax, dword [rbp + -8]
    push rax
    mov rax, 100
    mov rbx, rax
    pop rcx
    add rcx, rbx
    mov rax, rcx
    push rax
    lea rax, [rbp + -8]
    pop rbx
    mov [rax], ebx
    jmp _if_end_2
_if_false_2:
_if_end_2:
    lea rax, [rbp + -16]
    add rax, 4
    movsx rax, dword [rax]
    mov esi, eax
    movsx rax, dword [rbp + -8]
    mov edi, eax
    call calculate_score
    push rax
    lea rax, [rbp + -24]
    pop rbx
    mov [rax], ebx
    movsx rax, dword [rbp + -24]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    movsx rax, dword [rbp + -24]
    push rax
    mov rax, 150
    mov rbx, rax
    pop rcx
    cmp rcx, rax
    setge al
    movzx rax, al
    cmp rax, 0
    je _if_false_3
_if_true_3:
    mov rax, 1
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    jmp _if_end_3
_if_false_3:
    mov rax, 0
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
_if_end_3:
    mov rax, 0
.main_epilogue:
    leave
    ret

_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall
