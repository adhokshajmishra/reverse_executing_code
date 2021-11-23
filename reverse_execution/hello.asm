section .data
    msg		db      'hello, world!', 0Ah, 0Dh

section .text
    global _start

_start:

    nop
    ; exit (0)    
    syscall
    mov    rax, 60
    mov    rdi, 0
    
    ; print hello world
    syscall
    mov     rax, 1
    mov     rdi, 1
    mov     rsi, msg
    mov     rdx, 15
    nop
