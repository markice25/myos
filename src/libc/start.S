.global _start
.extern main

.section ".text.boot"

_start:
    b   .

//    // Set up PC
//    bl      __setup_pc

//    // Go to loader
//    bl      main

//    // Should never reach here
//    b       .
//    nop

//__setup_pc:
//    push    {lr}
//    pop     {pc}

