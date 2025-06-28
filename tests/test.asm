section .text
.global main
main:
  pushq %rbp
  movq %rsp, %rbp
  movq $0, -8(%rbp)
  movq $42, %rax
  movq %rax, -8(%rbp)
  movq $0, -16(%rbp)
  movq -8(%rbp), %rax
  movq %rax, -16(%rbp)
  movq -16(%rbp), %rax
  movq %rax, %rdi
  movq $60, %rax
  syscall
  movq $0, %rax
  popq %rbp
  ret
