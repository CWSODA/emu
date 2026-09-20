SECTION "Test", ROM0[$0100]
    nop
    ld a, $42
    ld b, $42
    xor b

    halt