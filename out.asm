section .text
global _start
_start:
  push rbp
  mov rbp, rsp
  mov qword [rbp + -8], 0
  mov rax, 0
  mov qword [rbp + -8], rax
  mov qword [rbp + -16], 0
  mov rax, qword [rbp + -8]
  mov qword [rbp + -16], rax
  mov rax, qword [rbp + -16]
  mov rdi, rax
  mov rax, 60
  syscall
  mov rax, 60
  xor rdi, rdi
  syscall
