global main
section .text

extern printf
extern scanf
buf: db "%.3lf", 10, 0
buf_scanf: db "%lf", 0

main:
    mov rax, 2309543223453
    mov rcx, 2309543223453
    mov rdx, 2309543223453
    mov rbx, 2309543223453
    mov rsp, 2309543223453
    mov rbp, 2309543223453
    mov rsi, 2309543223453
    mov rdi, 2309543223453
    mov r8, 2309543223453
    mov r9, 2309543223453
    mov r10, 2309543223453
    mov r11, 2309543223453
    mov r12, 2309543223453
    mov r13, 2309543223453
    mov r14, 2309543223453
    mov r15, 2309543223453

;     mov r8, rax
;     mov r9, rax
;     mov r10, rax
;     mov r11, rax
;     mov r12, rax
;     mov r13, rax
;     mov r14, rax
;     mov r15, rax
;
;     mov rax, rax
;     mov rcx, rax
;     mov rdx, rax
;     mov rbx, rax
;     mov rsp, rax
;     mov rbp, rax
;     mov rsi, rdi

      mov rax, r8
      mov rax, r9
      mov rax, r10
      mov rax, r11
      mov rax, r12
      mov rax, r13
      mov rax, r14
      mov rsp, r14

      sub rsp, 8
      sub rsp, 16
      sub rsp, 24
      sub rsp, 32
      sub rsp, 40
      sub rsp, 160
      sub rsp, 80000
      sub rsp, 1120

      push rbx
      push r11
      push r12
      push r13
      push r14
      push r15

      pop r15
      pop r14
      pop r13
      pop r12
      pop r11
      pop rbx
;
;     mov r8, r8
;     mov r9, r8
;     mov r10, r8
;     mov r11, r8
;     mov r12, r8
;     mov r13, r8
;     mov r14, r8
;     mov r15, r8

    add r8, rax
    add r9, rax
    add r10, rax
    add r11, rax
    add r12, rax
    add r13, rax
    add r14, rax

    add r15, rax
    add r15, rcx
    add r15, rdx
    add r15, rbx
    add r15, rsp
    add r15, rbp
    add r15, rsi
    add r15, rdi

    add rax, rax
    add rcx, rax
    add rdx, rax
    add rbx, rax
    add rsp, rax
    add rbp, rax
    add rsi, rdi

    add r8, r8
    add r9, r8
    add r10, r8
    add r11, r8
    add r12, r8
    add r13, r8
    add r14, r8
    add r15, r8

    push r8
    push r9
    push r10
    push r11
    pop r12
    pop r13
    pop r14
    pop r15

    push rax
    push rcx
    push rdx
    push rbx
    pop rsp
    pop rbp
    pop rsi

    push r8
    push r9
    push r10
    push r11
    pop r12
    pop r13
    pop r14
    pop r15


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






