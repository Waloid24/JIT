global _start
section .text


_start:
    mov rax, 100000000
    mov rbx, 150000000

    cmp rbx, rax
    je .equal
    mov rax, 0
    push rax
    jmp .after_equal
.equal:
    mov rax, 1
    push rax
.after_equal:

    pop rax
    
    mov rax, 0x3C		;exit64(rdi)
    xor rdi, rdi
    syscall




