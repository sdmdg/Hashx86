;
;  @file        loader.asm
;  @brief       Main loader of #x86
;  
;  @date        24/04/2025
;  @version     1.0.0-beta
;
;
; Refer to https://www.gnu.org/software/grub/manual/multiboot/multiboot.html;Header-layout for more info.
;
; ---------------------------------------------------------------------------------------------- ;
; Multiboot header (VBE VESA Graphics with 1152x864x32) NOTE : If you want to use VBE graphics,  ;
;                                                              uncomment this section            ;
; ---------------------------------------------------------------------------------------------- ;
FMBALIGN    equ 1<<0						; Align loaded modules on page boundaries            ;
MEMINFO     equ 1<<1						; Provide memory map                                 ;
VIDINFO     equ 1<<2						; OS wants video mode set                            ;
;                                                                                                ;
FLAGS       equ FMBALIGN | MEMINFO | VIDINFO; This is the multiboot 'flag' field                 ;
MAGIC       equ 0x1BADB002					; Magic Number                                       ;
CHECKSUM    equ -(MAGIC + FLAGS)			; Checksum                                           ;
;                                                                                                ;
; NOTE :                                                                                         ;
;     *) Yes, I hard-coded the VESA Graphics config into this assembly file because I'm lazy.    ;
;     *) If future me wants to make this configurable, too bad.                                  ;
;                                                                              - Past me         ;
;                                                                                  :)            ;
;                                                                                                ;
section .multiboot                                                                               ;
align 4                                                                                          ;
    dd MAGIC                             ; Store the magic number                                ;
    dd FLAGS                             ; Store the flags value                                 ;
    dd CHECKSUM                          ; Store the checksum value                              ;
;                                                                                                ;
    dd 0                                 ; Header Address                                        ;
    dd 0                                 ; Load Address                                          ;
    dd 0                                 ; Load end Address                                      ;
    dd 0                                 ; Bss end Address                                       ;
    dd 0                                 ; Entry Address                                         ;
;                                                                                                ;
    dd 0                                 ; Mode Type                                             ;
    dd 1152                              ; Width                                                 ;
    dd 864                               ; Height                                                ;
    dd 32                                ; Depth                                                 ;
; ---------------------------------------------------------------------------------------------- ;



; ---------------------------------------------------------------------------------------------- ;
; Multiboot header (VGA)  NOTE : If you want to use ugly VGA graphics, uncomment this section    ;
; ---------------------------------------------------------------------------------------------- ;
;FMBALIGN    equ 1<<0						; Align loaded modules on page boundaries            ;
;MEMINFO     equ 1<<1						; Provide memory map                                 ;
;VIDINFO     equ 1<<2						; OS wants video mode set                            ;
;                                                                                                ;
;FLAGS       equ FMBALIGN | MEMINFO | VIDINFO; This is the multiboot 'flag' field                ;
;MAGIC       equ 0x1BADB002					; Magic Number                                       ;
;CHECKSUM    equ -(MAGIC + FLAGS)			; Checksum                                           ;
;                                                                                                ;
;section .multiboot                                                                              ;
;align 4                                                                                         ;
;    dd MAGIC                     ; Store the magic number                                       ;
;    dd FLAGS                     ; Store the flags value                                        ;
;    dd CHECKSUM                  ; Store the checksum value                                     ;
; ---------------------------------------------------------------------------------------------- ;

; Define the text section where executable code is placed
section .text
extern kernelMain        ; Declare an external reference to the kernel's entry point function (kernelMain)
extern callConstructors  ; Declare an external reference for calling constructors (C++ global/static constructors)
global loader            ; Make the loader function globally accessible

; The loader function is the entry point executed after the bootloader loads the kernel
loader:
    mov esp, kernel_stack     ; Set the stack pointer to the beginning of the kernel stack
    call callConstructors     ; Call the constructor functions
    
    push eax                  ; Save the value of EAX register (for passing data)
    push ebx                  ; Save the value of EBX register (for passing data)

    call kernelMain           ; Call the kernel's main entry function

; Infinite loop to halt the CPU if something goes wrong
_stop:
    cli                       ; Clear interrupts
    hlt                       ; Halt the CPU
    jmp _stop                 ; Jump to _stop, causing an infinite loop

; Define the BSS section where uninitialized data
section .bss
resb 64*1024*1024;            ; Allocate 8 MB of space for the kernel's stack

kernel_stack:                 ; Label for the start of the kernel stack
