#ifndef _GLYCO_PINOUT_H
#define _GLYCO_PINOUT_H

#include <avr/io.h>

#include <stdint.h>
#include <stdbool.h>

// See doc/z80comp_pinout.svg and doc/Arduino-Mega-Pinout.jpg

#define PINOUT_LED_DDR DDRB
#define PINOUT_LED_PORT PORTB
#define PINOUT_LED_MASK (1 << PB7)

#define PINOUT_DATA_DDR DDRL
#define PINOUT_DATA_PORT PORTL
#define PINOUT_DATA_PIN PINL

#define PINOUT_ADDR_HI_DDR DDRC
#define PINOUT_ADDR_HI_PORT PORTC
#define PINOUT_ADDR_HI_PIN PINC

#define PINOUT_ADDR_LO_DDR DDRA
#define PINOUT_ADDR_LO_PORT PORTA
#define PINOUT_ADDR_LO_PIN PINA

#define PINOUT_FLASH_WE_DDR DDRG
#define PINOUT_FLASH_WE_PORT PORTG
#define PINOUT_FLASH_WE_MASK (1 << PG0)

#define PINOUT_RAM_WE_DDR DDRG
#define PINOUT_RAM_WE_PORT PORTG
#define PINOUT_RAM_WE_MASK (1 << PG2)

#define PINOUT_MEM_OE_DDR DDRG
#define PINOUT_MEM_OE_PORT PORTG
#define PINOUT_MEM_OE_MASK (1 << PG1)

#define PINOUT_BUSREQ_DDR DDRB
#define PINOUT_BUSREQ_PORT PORTB
#define PINOUT_BUSREQ_MASK (1 << PB0)

#define PINOUT_BUSACK_DDR DDRB
#define PINOUT_BUSACK_PORT PORTB
#define PINOUT_BUSACK_PIN PINB
#define PINOUT_BUSACK_MASK (1 << PB2)

enum pin_direction {
    PIN_INPUT = 0,
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
    PINOUT_ADDR_HI_DDR = -(int) direction;
    PINOUT_ADDR_LO_DDR = -(int) direction;
}

// Write a value to the data bus.
// Requires that the data bus DDR is set to output.
static inline void pinout_write_data(uint8_t data) {
    PINOUT_DATA_PORT = data;
}

// Read a value from the data bus.
// Requires that the data bus DDR is set to input.
static inline uint8_t pinout_read_data(void) {
    return PINOUT_DATA_PIN;
}

// Write a value to the address bus.
// Requires that the address bus DDR is set to output.
static inline void pinout_write_addr(uint16_t addr) {
    PINOUT_ADDR_HI_PORT = addr >> 8;
    PINOUT_ADDR_LO_PORT = addr & 0xFF;
}

// Read a value from the address bus.
// Requires that the address bus DDR is set to input.
static inline uint16_t pinout_read_addr(void) {
    return (PINOUT_ADDR_HI_PIN << 8) | PINOUT_ADDR_LO_PIN;
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
