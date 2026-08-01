bits 64

section .data
    align 32
    Q_ZERO: dq 1.0, 0.0, 0.0, 0.0 
    Q_ONE:  dq 0.0, 0.0, 1.0, 0.0 

    ; 2^63 as a double - used to scale the random integer
    SCALE:  dq 9223372036854775808.0 

section .text
    global q_measure

q_measure:
    ; Calculate P(1) = A1_re^2 + A1_im^2
    vmovupd ymm0, [r15 + rdi]
    vextractf128 xmm1, ymm0, 1
    vmulpd xmm1, xmm1, xmm1 
    vhaddpd xmm1, xmm1, xmm1

    ; Get Hardware Random Number
    .retry:
        rdrand rax
        jnc .retry

    ; Convert to float in range [0, 1]
    ; Shift right by 1 to clear the sign bit (makes it a positive 63-bit int)
    shr rax, 1
    vcvtsi2sd xmm2, xmm2, rax
    vdivsd xmm2, xmm2, [rel SCALE]

    ; Compare Probability with Random Number
    ucomisd xmm1, xmm2
    ja .measured_one

.measured_zero:
    vmovupd ymm0, [rel Q_ZERO]
    vmovupd [r15 + rdi], ymm0
    xor rax, rax
    ret

.measured_one:
    vmovupd ymm0, [rel Q_ONE]
    vmovupd [r15 + rdi], ymm0
    mov rax, 1
    ret
