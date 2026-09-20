SECTION "Test", ROM0[$0100]
    nop

    ld hl, $c000
    ld [hl], $67

    halt