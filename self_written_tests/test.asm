SECTION "Test", ROM0[$0100]
    nop

    ld sp, $ff
    ld hl, sp + 1

    ld sp, $ff
    add sp, 1
    ld sp, $ff
    add sp, -1

    halt