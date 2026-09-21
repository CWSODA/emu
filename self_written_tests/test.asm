SECTION "Test", ROM0[$0100]
    nop

    ld hl, $7575
    ld sp, $1110
    ld hl, sp + $f0

    halt