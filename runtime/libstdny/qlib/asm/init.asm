bits 64

section .data
    align 32
    QINIT_AMP: dq 1.000, 0.000, 0.000, 0.000

section .text
    global q_init

q_init:
    vxorpd ymm0, ymm0, ymm0
    vmovapd ymm0, [rel QINIT_AMP]
    vmovupd [rdi], ymm0 
    ret

; usage:
; lea rdi, [r15 + 32]
; call q_init
