%define IRQ_BASE 0x20

section .text

extern _ZN16InterruptManager15handleInterruptEhj
extern _ZN16InterruptManager15handleExceptionEhj

;----------------------------------------
; Macro for exceptions without error code
;----------------------------------------
%macro HandleExceptionWithoutError 1
global _ZN16InterruptManager19HandleException%1Ev
_ZN16InterruptManager19HandleException%1Ev:
    mov byte [interruptnumber], %1
    push dword 0            ; Push dummy error code
    jmp exc_common_handler
%endmacro

;----------------------------------------
; Macro for exceptions with error code
;----------------------------------------
%macro HandleExceptionWithError 1
global _ZN16InterruptManager19HandleException%1Ev
_ZN16InterruptManager19HandleException%1Ev:
    mov byte [interruptnumber], %1
    jmp exc_common_handler
%endmacro

;----------------------------------------
; Macro for Interrupt Requests
;----------------------------------------
%macro HandleInterruptRequest 1
global _ZN16InterruptManager26HandleInterruptRequest%1Ev
_ZN16InterruptManager26HandleInterruptRequest%1Ev:
    mov byte [interruptnumber], %1 + IRQ_BASE
    push dword 0            ; Dummy error code for interrupts
    jmp intr_common_handler
%endmacro


;---------------------
; Exceptions
;---------------------
HandleExceptionWithoutError 0x00   ; Divide Error
HandleExceptionWithoutError 0x01   ; Debug
HandleExceptionWithoutError 0x02   ; NMI
HandleExceptionWithoutError 0x03   ; Breakpoint
HandleExceptionWithoutError 0x04   ; Overflow
HandleExceptionWithoutError 0x05   ; BOUND Range Exceeded
HandleExceptionWithoutError 0x06   ; Invalid Opcode
HandleExceptionWithoutError 0x07   ; Device Not Available
HandleExceptionWithError    0x08   ; Double Fault
HandleExceptionWithoutError 0x09   ; Coprocessor Segment Overrun
HandleExceptionWithError    0x0A   ; Invalid TSS
HandleExceptionWithError    0x0B   ; Segment Not Present
HandleExceptionWithError    0x0C   ; Stack-Segment Fault
HandleExceptionWithError    0x0D   ; General Protection Fault
HandleExceptionWithError    0x0E   ; Page Fault
HandleExceptionWithoutError 0x0F   ; Reserved
HandleExceptionWithoutError 0x10   ; x87 FPU Error
HandleExceptionWithError    0x11   ; Alignment Check
HandleExceptionWithoutError 0x12   ; Machine Check
HandleExceptionWithoutError 0x13   ; SIMD Floating-Point Exception

;---------------------
; Interrupt Requests
;---------------------
HandleInterruptRequest 0x00
HandleInterruptRequest 0x01
HandleInterruptRequest 0x02
HandleInterruptRequest 0x03
HandleInterruptRequest 0x04
HandleInterruptRequest 0x05
HandleInterruptRequest 0x06
HandleInterruptRequest 0x07
HandleInterruptRequest 0x08
HandleInterruptRequest 0x09
HandleInterruptRequest 0x0A
HandleInterruptRequest 0x0B
HandleInterruptRequest 0x0C
HandleInterruptRequest 0x0D
HandleInterruptRequest 0x0E
HandleInterruptRequest 0x0F
HandleInterruptRequest 0x31

HandleInterruptRequest 0x80
HandleInterruptRequest 0x81


;------------------------
; Common exception handler
;------------------------
exc_common_handler:
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax

    push esp
    push dword [interruptnumber]
    call _ZN16InterruptManager15handleExceptionEhj
    mov esp, eax

    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    add esp, 4
    iret


;------------------------
; Common interrupt handler
;------------------------
intr_common_handler:
    push ebp
    push edi
    push esi
    push edx
    push ecx
    push ebx
    push eax

    push esp
    push dword [interruptnumber]
    call _ZN16InterruptManager15handleInterruptEhj
    mov esp, eax

    pop eax
    pop ebx
    pop ecx
    pop edx
    pop esi
    pop edi
    pop ebp
    add esp, 4
    iret


global _ZN16InterruptManager22IgnoreInterruptRequestEv
_ZN16InterruptManager22IgnoreInterruptRequestEv:
    iret


section .data
interruptnumber: db 0
