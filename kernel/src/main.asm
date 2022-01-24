rst00:
    jp boot

.fill 0x08-$
rst08:
    ret

.fill 0x10-$
rst10:
    ret

.fill 0x18-$
rst18:
    ret

.fill 0x20-$
rst20:
    ret

.fill 0x28-$
rst28:
    ret

.fill 0x30-$
rst30:
    ret

.fill 0x38-$
rst38:
    ret

boot:
    ld h, 0
    ld l, 0
.loop:
    ld a, h
    or 0x80
    ld h, a
    ld (hl), 123
    inc hl
    jp .loop
