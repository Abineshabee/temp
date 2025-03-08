ENTRY(_start)

PHDRS
{
    text PT_LOAD FLAGS(5);  /* Read + Execute */
    data PT_LOAD FLAGS(6);  /* Read + Write */
}

SECTIONS
{
    . = 1M;

    .text : {
        *(.multiboot)
        *(.text)
    } :text

    .rodata : { *(.rodata) } :text

    .data : { *(.data) } :data
    .bss : { *(.bss) } :data
}

