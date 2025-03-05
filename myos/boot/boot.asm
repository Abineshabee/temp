BITS 32
section .multiboot
align 4
dd 0x1BADB002
dd 0x00
dd - (0x1BADB002 + 0x00)

section .text
global _start
extern kernel_main

_start:
    cli
    mov esp, stack
    call kernel_main
    hlt

section .bss
align 16
stack:
    resb 4096

section .note.GNU-stack noalloc noexec nowrite progbits  ; Fix GNU-stack warning

