.set IRQ_BASE, 0x20

.section .text

.extern _ZN16InterruptManager15handleInterruptEhj
.extern _ZN16InterruptManager15handleExceptionEhj

/* Macro for exceptions without error code (push dummy) */
.macro HandleExceptionWithoutError num
.global _ZN16InterruptManager19HandleException\num\()Ev
_ZN16InterruptManager19HandleException\num\()Ev:
    movb $\num, (interruptnumber)
    pushl $0      # Push dummy error code
    jmp exc_common_handler
.endm

/* Macro for exceptions with error code (no push) */
.macro HandleExceptionWithError num
.global _ZN16InterruptManager19HandleException\num\()Ev
_ZN16InterruptManager19HandleException\num\()Ev:
    movb $\num, (interruptnumber)
    jmp exc_common_handler
.endm

/* Macro for Interrupt Requests */
.macro HandleInterruptRequest num
.global _ZN16InterruptManager26HandleInterruptRequest\num\()Ev
_ZN16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)
    pushl $0      # Dummy error code for interrupts
    jmp intr_common_handler
.endm

# Exceptions
HandleExceptionWithoutError 0x00   # Divide Error
HandleExceptionWithoutError 0x01   # Debug
HandleExceptionWithoutError 0x02   # NMI
HandleExceptionWithoutError 0x03   # Breakpoint
HandleExceptionWithoutError 0x04   # Overflow
HandleExceptionWithoutError 0x05   # BOUND Range Exceeded
HandleExceptionWithoutError 0x06   # Invalid Opcode
HandleExceptionWithoutError 0x07   # Device Not Available
HandleExceptionWithError    0x08   # Double Fault
HandleExceptionWithoutError 0x09   # Coprocessor Segment Overrun
HandleExceptionWithError    0x0A   # Invalid TSS
HandleExceptionWithError    0x0B   # Segment Not Present
HandleExceptionWithError    0x0C   # Stack-Segment Fault
HandleExceptionWithError    0x0D   # General Protection Fault
HandleExceptionWithError    0x0E   # Page Fault
HandleExceptionWithoutError 0x0F   # Reserved
HandleExceptionWithoutError 0x10   # x87 FPU Error
HandleExceptionWithError    0x11   # Alignment Check
HandleExceptionWithoutError 0x12   # Machine Check
HandleExceptionWithoutError 0x13   # SIMD Floating-Point Exception

# Interrupt Requests
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

# Common exception handling
exc_common_handler:
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax

    pushl %esp
    push (interruptnumber)
    call _ZN16InterruptManager15handleExceptionEhj
    mov %eax, %esp

    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    add $4, %esp
    iret

# Common interrupt handling
intr_common_handler:
    pushl %ebp
    pushl %edi
    pushl %esi
    pushl %edx
    pushl %ecx
    pushl %ebx
    pushl %eax

    pushl %esp
    push (interruptnumber)
    call _ZN16InterruptManager15handleInterruptEhj
    mov %eax, %esp

    popl %eax
    popl %ebx
    popl %ecx
    popl %edx
    popl %esi
    popl %edi
    popl %ebp
    add $4, %esp
    iret

.global _ZN16InterruptManager22IgnoreInterruptRequestEv
_ZN16InterruptManager22IgnoreInterruptRequestEv:
    iret

.section .data
    interruptnumber: .byte 0
