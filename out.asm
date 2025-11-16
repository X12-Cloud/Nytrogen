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
    sub rsp, 8
    mov rax, 10
    push rax
    mov rax, [rbp + -8]
    mov rax, [rax + 0]
    pop rbx
    mov [rax], rbx
    mov rax, 20
    push rax
    mov rax, [rbp + -8]
    mov rax, [rax + 4]
    pop rbx
    mov [rax], rbx
    mov rax, [rbp + -8]
    mov rax, [rax + 0]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    mov rax, [rbp + -8]
    mov rax, [rax + 4]
    mov rsi, rax
    lea rdi, [rel _print_int_format]
    xor rax, rax
    call printf
    mov rax, 0
.main_epilogue:
    mov rsp, rbp
    pop rbp
    ret

_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall
