# Define constants for the multiboot header
.set MAGIC, 0x1badb002           # Magic number for multiboot header (used by bootloader to identify a valid kernel)
.set FLAGS, (1<<0 | 1<<1)        # Flags used by the bootloader to define certain behaviors (e.g. memory map, module info)
.set CHECKSUM, -(MAGIC + FLAGS)  # The checksum is the negative sum of the MAGIC and FLAGS to ensure validity

# Define the multiboot section that will be read by the bootloader
.section .multiboot
    .long MAGIC       # Store the magic number
    .long FLAGS       # Store the flags value
    .long CHECKSUM    # Store the checksum value

# Define the text section where executable code is placed
.section .text
.extern kernelMain        # Declare an external reference to the kernel's entry point function (kernelMain)
.extern callConstructors  # Declare an external reference for calling constructors (e.g., C++ global/static constructors)
.global loader            # Make the loader function globally accessible

# The loader function is the entry point executed after the bootloader loads the kernel
loader:
    mov $kernel_stack, %esp   # Set the stack pointer to the beginning of the kernel stack
    call callConstructors     # Call the constructor functions (initialization code)
    
    push %eax                 # Save the value of EAX register (for passing data)
    push %ebx                 # Save the value of EBX register (for passing data)
    
    call kernelMain           # Call the kernel's main entry function

# Infinite loop to halt the CPU if something goes wrong
_stop:
    cli                       # Clear interrupts
    hlt                       # Halt the CPU
    jmp _stop                 # Jump to _stop, causing an infinite loop

# Define the BSS section where uninitialized data
.section .bss
.space 4*1024*1024;           # Allocate 4 MB of space for the kernel's stack

kernel_stack:                 # Label for the start of the kernel stack
