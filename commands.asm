global _start

section .text

_start:

    mulpd xmm0, xmm0
    mulpd xmm0, xmm1
    mulpd xmm0, xmm2

    mulpd xmm0, xmm0
    mulpd xmm1, xmm0
    mulpd xmm2, xmm0

    divpd xmm0, xmm0
    divpd xmm0, xmm1
    divpd xmm0, xmm2

    divpd xmm0, xmm0
    divpd xmm1, xmm0
    divpd xmm2, xmm0

    mov rax, 0x3C
    xor rdi, rdi
    syscall
