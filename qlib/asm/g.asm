bits 64

section .data
    align 32
    H_C:    dq 0.70710678, 0.70710678, 0.70710678, 0.70710678
    Z_MASK: dq 1.0, 1.0, -1.0, -1.0

    S_COS:  dq 0.0
    S_SIN:  dq 1.0
    T_COS:  dq 0.70710678
    T_SIN:  dq 0.70710678

section .text
    global q_h, q_x, q_z, q_s, q_t, q_rz

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

q_z:
    vmovupd ymm0, [r15 + rdi]
    vmulpd  ymm0, ymm0, [rel Z_MASK]
    vmovupd [r15 + rdi], ymm0 
    ret

q_s:
    movsd xmm0, [rel S_COS]
    movsd xmm1, [rel S_SIN]
    jmp q_rz

q_t:
    movsd xmm0, [rel T_COS]
    movsd xmm1, [rel T_SIN]
    jmp q_rz

; q_rz: rdi=offset, xmm0=cos, xmm1=sin
q_rz:
    vunpcklpd xmm4, xmm0, xmm0
    vunpcklpd xmm5, xmm1, xmm1
    vmovupd ymm0, [r15 + rdi]
    vextractf128 xmm2, ymm0, 1
    vpermilpd xmm3, xmm2, 0b01
    vmulpd xmm2, xmm2, xmm4
    vmulpd xmm3, xmm3, xmm5
    vaddsubpd xmm2, xmm2, xmm3
    vinsertf128 ymm0, ymm0, xmm2, 1
    vmovupd [r15 + rdi], ymm0
    ret
