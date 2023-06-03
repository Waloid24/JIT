global _start

section .text

_start:

    sub rax, rax

    sub r8, rax

    sub rax, r8

    sub r8, r8

    mov rax, 0x3C
    xor rdi, rdi
    syscall
