ENTRY(_start)

SECTIONS
{
    . = 0x10000;
    __start = .;

    .text :
    {
        __text_start = .;
        *(.text.boot)
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        __text_end = .;
    }

    .data :
    {
        __data_start = .;
        *(.data)
        *(.data*)
        __data_end = .;
    }

    .bss :
    {
        __bss_start = .;
        *(.bss)
        *(.bss*)
        *(.*bss)
        *(.*bss*)
        *(COMMON)
        __bss_end = .;
    }

    .sdata :
    {
         *(.sdata)
         *(.sdata.*)
    }

    .scommon :
    {
        *(.scommon)
        *(.scommon.*)
    }

/*    .fdt :
    {
        __fdt_start = .;
        LONG(0x0);
        __fdt_end = .;
    }

    .coreimg :
    {
        __coreimg_start = .;
        LONG(0x0);
        __coreimg_end = .;
    }
*/
    /DISCARD/ :
    {
        *(.pdr)
        *(.comment)
        *(.gnu*)
    }

    __end = .;
}

