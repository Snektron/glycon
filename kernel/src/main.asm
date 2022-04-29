PORT_PIO_A_DATA    .equ 0x00
PORT_PIO_B_DATA    .equ 0x01
PORT_PIO_A_CONTROL .equ 0x02
PORT_PIO_B_CONTROL .equ 0x03

PIO_CMD_SET_MODE .equ 0x0F

PIO_MODE_OUTPUT .equ 0x00
PIO_MODE_INPUT  .equ 0x40
PIO_MODE_BIDIR  .equ 0x80
PIO_MODE_CTRL   .equ 0xC0

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
    ; Disable interrupts
    di
    ; Set the page pio to manual mode
    ld a, PIO_CMD_SET_MODE | PIO_MODE_CTRL
    out (PORT_PIO_B_CONTROL), a
    ; Define all pins of this port as output
    xor a, a
    out (PORT_PIO_B_CONTROL), a
    ; Select page 0x8 (the first ram page) and assign it to slots 2 and 3 (addresses 0x8000, 0xC000).
    ld a, 0x8
    out (PORT_PIO_B_DATA), a
    ; Write a bunch of stuff.
    ld h, 0
    ld l, 0
.loop:
    ld a, h
    or 0x80
    ld h, a
    ld (hl), 123
    inc hl
    jp .loop
