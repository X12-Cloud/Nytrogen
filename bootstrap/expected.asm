section .data
  _print_int_format db "%d", 10, 0
  _print_str_format db "%s", 10, 0
  _print_char_format db "%c", 10, 0
section .text
global _start
extern printf

printer:
    push rbp
    mov rbp, rsp
    mov rax, [rbp + 16]           ; rax gets the string address passed to printer
    mov rsi, rax                  ; rsi (printf's 2nd arg) holds the string address
    lea rdi, [rel _print_str_format] ; <--- **THIS IS THE CRITICAL CHANGE**
                                  ; rdi (printf's 1st arg) now gets address of "%s", 10, 0
    xor rax, rax
    call printf
    mov rsp, rbp
    pop rbp
    ret


main:
    push rbp
    mov rbp, rsp
section .data
_str_0 db "hello wrld", 0
section .text
    lea rax, [rel _str_0]
    push rax
    call printer
    add rsp, 8
.main_epilogue:
    mov rsp, rbp
    pop rbp
    ret

_start:
  call main
  mov rdi, rax
  mov rax, 60
  syscall
