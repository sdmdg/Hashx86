ENTRY(_start)
OUTPUT_FORMAT(elf32-i386)

SECTIONS
{
    . = 0x1900F37C;

    .text : ALIGN(4)
    {
        *(.text*)
        . = ALIGN(4096);
    }

    .rodata : ALIGN(4)
    {
        *(.rodata*)
    }

    .init_array : ALIGN(4)
    {
        start_ctors = .;
        KEEP(*(.init_array))
        KEEP(*(SORT_BY_INIT_PRIORITY(.init_array.*)))
        end_ctors = .;
    }

    .data : ALIGN(4)
    {
        *(.data)
        . = ALIGN(4096);
    }

    .bss : ALIGN(4)
    {
        *(.bss)
        *(COMMON)
        . = ALIGN(4096);
    }

    /DISCARD/ :
    {
        *(.fini_array*)
        *(.comment)
    }
}
