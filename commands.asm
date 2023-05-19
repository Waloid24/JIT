global main
section .text

extern printf
extern scanf
buf: db "%.3lf", 10, 0
buf_scanf: db "%lf", 0

main:
    mov rbp, rsp
    and rsp, 0xffffffffffffff10

    mov rax, 1
    mov rdi, buf_scanf
    call scanf
    mov rsp, rbp
    cvtsd2si rax, xmm0
    push rax



;     cvtsi2sd xmm0, rax
    mov rax, 1000
    cvtsi2sd xmm1, rax
    divpd xmm0, xmm1

    mov rbp, rsp
    and rsp, 0xffffffffffffff10
    mov rdi, buf
    mov rax, 1
    call printf

    mov rsp, rbp
    
    mov rax, 0x3C		;exit64(rdi)
    xor rdi, rdi
    syscall






