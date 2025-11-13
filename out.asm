section .data
  _print_int_format db "%d", 10, 0
  _print_str_format db "%s", 10, 0
  _print_char_format db "%c", 10, 0
section .text
global _start
extern printf
main:
    push rbp
    mov rbp, rsp
    sub rsp, 20
    mov rax, 10
    push rax
    mov rax, 0
    mov rbx, rax
    mov rax, [rbp + -20]
    imul rbx, 8
    add rax, rbx
    mov rax, [rax]
    pop rbx
    mov [rax], rbx
    mov rax, 20
    push rax
    mov rax, 1
    mov rbx, rax
    mov rax, [rbp + -20]
    imul rbx, 8
    add rax, rbx
    mov rax, [rax]
    pop rbx
    mov [rax], rbx
    mov rax, 0
    mov rbx, rax
    mov rax, [rbp + -20]
    imul rbx, 8
    add rax, rbx
    mov rax, [rax]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    mov rax, 1
    mov rbx, rax
    mov rax, [rbp + -20]
    imul rbx, 8
    add rax, rbx
    mov rax, [rax]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    mov rax, 0
    ret
.main_epilogue:
    mov rsp, rbp
    pop rbp
    ret

_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall
