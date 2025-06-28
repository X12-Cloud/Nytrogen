section .data
    msg db "Hello, world!", 0x0A ; string and newline
    len equ $ - msg             ; length of string

section .text
    global _start

_start:
    ; write(1, msg, len)
    mov eax, 4      ; sys_write (Linux 32-bit, but works for basic test)
    mov ebx, 1      ; stdout
    mov ecx, msg    ; message address
    mov edx, len    ; message length
    int 0x80        ; call kernel

    ; exit(0)
    mov eax, 1      ; sys_exit
    mov ebx, 0      ; exit code 0
    int 0x80        ; call kernel
