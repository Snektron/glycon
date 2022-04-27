#ifndef _GLYCO_PINOUT_H
#define _GLYCO_PINOUT_H

#include <avr/io.h>

#include <stdint.h>
#include <stdbool.h>

#include "util.h"

// See doc/z80comp_pinout.svg and doc/Arduino-Mega-Pinout.jpg

#define PINOUT_LED_DDR DDRB
#define PINOUT_LED_PORT PORTB
#define PINOUT_LED_MASK (1 << PB7)

// Pins A0, A1, A2, A10, A3, A4, A11, A5
#define PINOUT_ADDR_A_DDR DDRA
#define PINOUT_ADDR_A_PORT PORTA
#define PINOUT_ADDR_A_PIN PINA

// Pins PG1, PG0, A12, A13, A7, A8, A6, A9
#define PINOUT_ADDR_B_DDR DDRC
#define PINOUT_ADDR_B_PORT PORTC
#define PINOUT_ADDR_B_PIN PINC

#define PINOUT_FLASH_WE_DDR DDRD
#define PINOUT_FLASH_WE_PORT PORTD
#define PINOUT_FLASH_WE_MASK (1 << PD7)

#define PINOUT_RAM_WE_DDR DDRG
#define PINOUT_RAM_WE_PORT PORTG
#define PINOUT_RAM_WE_MASK (1 << PG2)

#define PINOUT_IOREQ_DDR DDRG
#define PINOUT_IOREQ_PORT PORTG
#define PINOUT_IOREQ_MASK (1 << PG0)

#define PINOUT_MEM_OE_DDR DDRG
#define PINOUT_MEM_OE_PORT PORTG
#define PINOUT_MEM_OE_MASK (1 << PG1)

#define PINOUT_DATA_DDR DDRL
#define PINOUT_DATA_PORT PORTL
#define PINOUT_DATA_PIN PINL

#define PINOUT_M1_DDR DDRB
#define PINOUT_M1_PORT PORTB
#define PINOUT_M1_PIN PINB
#define PINOUT_M1_MASK (1 << PB3)

#define PINOUT_RESET_DDR DDRB
#define PINOUT_RESET_PORT PORTB
#define PINOUT_RESET_MASK (1 << PB1)

#define PINOUT_BUSREQ_DDR DDRB
#define PINOUT_BUSREQ_PORT PORTB
#define PINOUT_BUSREQ_MASK (1 << PB2)

#define PINOUT_BUSACK_DDR DDRB
#define PINOUT_BUSACK_PORT PORTB
#define PINOUT_BUSACK_PIN PINB
#define PINOUT_BUSACK_MASK (1 << PB0)

// This enumeration is used to indicate that a pin should be set to a particular mode.
enum pin_direction {
    // Indicates that a pin or group of pins should be set to INPUT mode.
    PIN_INPUT = 0,
    // Indicates that pin or group of pins should be set to OUTPUT mode.
    PIN_OUTPUT = 1
};

// Set the direction of the data pins.
// 0 means input, 1 is output.
static inline void pinout_set_data_ddr(enum pin_direction direction) {
    PINOUT_DATA_DDR = -(int) direction;
}

// Set the direction of the address pins.
// 0 means input, 1 is output.
static inline void pinout_set_addr_ddr(enum pin_direction direction) {
    PINOUT_ADDR_A_DDR = -(int) direction;
    PINOUT_ADDR_B_DDR = -(int) direction;
}

// Write a value to the data bus.
// Requires that the data bus DDR is set to output.
static inline void pinout_write_data(uint8_t data) {
    PINOUT_DATA_PORT = util_bit_reverse8(data);
}

// Read a value from the data bus.
// Requires that the data bus DDR is set to input.
static inline uint8_t pinout_read_data(void) {
    return util_bit_reverse8(PINOUT_DATA_PIN);
}

// Write a value to the address bus.
// Requires that the address bus DDR is set to output.
static inline void pinout_write_addr(uint16_t addr) {
    uint8_t a = 0;
    a |= ((addr >>  0) & 0x7) << 0; // A0-2
    a |= ((addr >> 10) & 0x1) << 3; // A10
    a |= ((addr >>  3) & 0x3) << 4; // A3-4
    a |= ((addr >> 11) & 0x1) << 6; // A11
    a |= ((addr >>  5) & 0x1) << 7; // A5

    uint8_t b = 0;
    b |= ((addr >> 14) & 0x3) << 6; // PG0-1
    b |= ((addr >> 12) & 0x1) << 5; // A12
    b |= ((addr >> 13) & 0x1) << 4; // A13
    b |= ((addr >>  7) & 0x1) << 3; // A7
    b |= ((addr >>  8) & 0x1) << 2; // A8
    b |= ((addr >>  6) & 0x1) << 1; // A6
    b |= ((addr >>  9) & 0x1) << 0; // A9

    PINOUT_ADDR_A_PORT = a;
    PINOUT_ADDR_B_PORT = b;
}

// Read a value from the address bus.
// Requires that the address bus DDR is set to input.
static inline uint16_t pinout_read_addr(void) {
    uint8_t a = PINOUT_ADDR_A_PIN;
    uint8_t b = PINOUT_ADDR_B_PIN;

    uint16_t addr = 0;
    addr |= ((a >> 0) & 0x7) <<  0; // A0-2
    addr |= ((a >> 3) & 0x1) << 10; // A10
    addr |= ((a >> 4) & 0x3) <<  3; // A3-4
    addr |= ((a >> 6) & 0x1) << 11; // A11
    addr |= ((a >> 7) & 0x1) <<  5; // A5

    addr |= ((b >> 6) & 0x3) << 14; // PG0-1
    addr |= ((b >> 5) & 0x1) << 12; // A12
    addr |= ((b >> 4) & 0x1) << 13; // A13
    addr |= ((b >> 3) & 0x1) <<  7; // A7
    addr |= ((b >> 2) & 0x1) <<  8; // A8
    addr |= ((b >> 1) & 0x1) <<  6; // A6
    addr |= ((b >> 0) & 0x1) <<  9; // A9

    return addr;
}

// Enable or disable output from both the ram and flash chip.
// When disabled, this pulls the MEM_OE pin HIGH.
// Requires that MEM_OE DDR is set to output.
static inline void pinout_enable_mem_output(bool enable) {
    if (enable) {
        PINOUT_MEM_OE_PORT &= ~PINOUT_MEM_OE_MASK;
    } else {
        PINOUT_MEM_OE_PORT |= PINOUT_MEM_OE_MASK;
    }
}

#endif
