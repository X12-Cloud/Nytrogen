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
    mov rax, 123
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
