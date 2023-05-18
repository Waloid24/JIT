
section .text
global _start
global main
extern printf

buf: times 128 db 0
printf_buf: db "mul = %ld", 10, 0

main:
; _start:
    ; mov rax, buf
    ; mov rcx, buf
    ; mov rdx, buf
    ; mov rbx, buf
    ; mov rsp, buf
    ; mov rbp, buf
    ; mov rsi, buf
    ; mov rdi, buf
    ; mov r8, buf
    ; mov r9, buf
    ; mov r10, buf
    ; mov r11, buf
    ; mov r12, buf
    ; mov r13, buf
    ; mov r14, buf
    ; mov r15, buf

;     push rax
;     push rsi
;     push rdi

    pop rax
    cvtsi2sd xmm1, rax
    
    pop rax
    cvtsi2sd xmm0, rax

    mov rax, 1000
    cvtsi2sd xmm2, rax

    divpd xmm0, xmm1
    mulpd xmm0, xmm2


    cvtsd2si rax, xmm0
    push rax
;     mov rdi, printf_buf
;     xor rax, rax
;     call printf

;     pop rdi
;     pop rsi
;     pop rax



    
    mov rax, 0x3C		;exit64(rdi)
    xor rdi, rdi
    syscall




