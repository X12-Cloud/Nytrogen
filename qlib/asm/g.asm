bits 64

section .data
    align 32
    H_C: dq 0.70710678, 0.70710678, 0.70710678, 0.70710678

section .text
    global q_h
    global q_x

q_h:
    vmovupd ymm0, [r15 + rdi]
    vperm2f128 ymm1, ymm0, ymm0, 1
    vaddpd  ymm2, ymm0, ymm1
    vsubpd  ymm3, ymm0, ymm1
    vbroadcastsd ymm4, [rel H_C]
    vmulpd  ymm2, ymm2, ymm4
    vmulpd  ymm3, ymm3, ymm4
    vblendpd ymm0, ymm2, ymm3, 12
    vmovupd [r15 + rdi], ymm0
    ret

q_x:
    vmovupd ymm0, [r15 + rdi]
    vperm2f128 ymm1, ymm0, ymm0, 1
    vmovupd [r15 + rdi], ymm1
    ret
