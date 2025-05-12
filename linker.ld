ENTRY(loader)
OUTPUT_FORMAT(elf32-i386)
OUTPUT_ARCH(i386:i386)

SECTIONS
{
    __kernel_section_start = .;
    . = 0x0100000;
    .text : ALIGN(4)
    {
        __kernel_text_section_start = .;
        *(.multiboot)  /* Must be in first 8KB of file */
        code = .; _code = .; __code = .;
        *(.text*)
        . = ALIGN(4096);
        __kernel_text_section_end = .;
    }

    .data : ALIGN(4)
    {
        __kernel_data_section_start = .;
        data = .; _data = .; __data = .;
        *(.data)
        . = ALIGN(4096);
        __kernel_data_section_end = .;
    }

    .init_array : ALIGN(4)
    {
        start_ctors = .;
        KEEP(*(.init_array))
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        end_ctors = .;
    }

    .rodata : ALIGN(4)
    {
        __kernel_rodata_section_start = .;
        *(.rodata)
        __kernel_rodata_section_end = .;
    }

    .bss : ALIGN(4)
    {
        __kernel_bss_section_start = .;
        bss = .; _bss = .; __bss = .;
        *(.bss)
        *(COMMON)
        . = ALIGN(4096);
        __kernel_bss_section_end = .;
    }

    /DISCARD/ :
    {
        *(.fini_array*)
        *(.comment)
    }
    
    end = .; _end = .; __end = .;
    __kernel_section_end = .;
}
