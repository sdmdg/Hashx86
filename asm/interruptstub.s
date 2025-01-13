.set IRQ_BASE, 0x20             # Define the base value for hardware interrupt requests (IRQs).

.section .text

.extern _ZN16InterruptManager15handleInterruptEhj
# Declare an external function `handleInterrupt` with Itanium C++ ABI mangling.
# This function will handle the actual interrupt in higher-level code.

.macro HandleException num
# Macro to define exception handlers for a specific exception number.
.global _ZN16InterruptManager16HandleException\num\()Ev
# Declare the global symbol for the exception handler function.
_ZN16InterruptManager16HandleException\num\()Ev:
    movb $\num, (interruptnumber)       # Store the exception number in the `interruptnumber` variable.
    jmp int_bottom                      # Jump to the shared interrupt handling code.
.endm

.macro HandleInterruptRequest num
# Macro to define interrupt request (IRQ) handlers for specific IRQ numbers.
.global _ZN16InterruptManager26HandleInterruptRequest\num\()Ev
# Declare the global symbol for the IRQ handler function.
_ZN16InterruptManager26HandleInterruptRequest\num\()Ev:
    movb $\num + IRQ_BASE, (interruptnumber)    # Store the IRQ number (offset by IRQ_BASE) in the `interruptnumber` variable.
    jmp int_bottom                              # Jump to the shared interrupt handling code.
.endm

# Define handlers for specific IRQs using the `HandleInterruptRequest` macro.
HandleInterruptRequest 0x00  # IRQ 0: Timer interrupt.
HandleInterruptRequest 0x01  # IRQ 1: Keyboard interrupt.
# :
# :
# :
HandleInterruptRequest 0x0C  # IRQ 12: Mouse interrupt.

int_bottom:
# Shared code for handling interrupts.
    pusha
    # Save all general-purpose registers on the stack.
    pushl %ds
    pushl %es
    pushl %fs
    pushl %gs
    # Save segment registers on the stack.

    pushl %esp
    # Pass the current stack pointer as a parameter.
    push (interruptnumber)
    # Pass the interrupt number as a parameter.
    call _ZN16InterruptManager15handleInterruptEhj
    # Call the external `handleInterrupt` function.
    movl %eax, %esp
    # Restore the stack pointer from the return value of the function.

    popl %gs
    popl %fs
    popl %es
    popl %ds
    # Restore segment registers from the stack.
    popa
    # Restore general-purpose registers from the stack.

.global _ZN16InterruptManager22IgnoreInterruptRequestEv
# Declare a global symbol for ignoring interrupt requests.
_ZN16InterruptManager22IgnoreInterruptRequestEv:
    iret                        # Return from the interrupt without doing anything.

.section .data
# Define the data section.
    interruptnumber: .byte 0    # Reserve a single byte to store the current interrupt number.
