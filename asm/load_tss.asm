global tss_flush
tss_flush:
    mov ax, 0x28      ; 0x28 is the offset in GDT (Index 5 * 8)
    ltr ax            ; Load Task Register
    ret
