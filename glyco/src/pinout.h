#ifndef _GLYCON_COPROCESSOR_PINOUT_H
#define _GLYCON_COPROCESSOR_PINOUT_H

#include <avr/io.h>

#include <stdint.h>
#include <stdbool.h>

#define PINOUT_LED_DDR DDRB
#define PINOUT_LED_PORT PORTB
#define PINOUT_LED_MASK (1 << PB7)

// Data pin definitions

#define PINOUT_DATA_DDR DDRL
#define PINOUT_DATA_PORT PORTL

// Address pin definitions

#define PINOUT_ADDR_HI_DDR DDRA
#define PINOUT_ADDR_HI_PORT PORTA

#define PINOUT_ADDR_LO_DDR DDRC
#define PINOUT_ADDR_LO_PORT PORTC

enum pin_direction {
    PIN_INPUT = 0,
    PIN_OUTPUT = 1
};

// Set the direction of the data pins.
// 0 means input, 1 is output.
void pinout_set_data_ddr(enum pin_direction direction);

// Set the direction of the address pins.
// 0 means input, 1 is output.
void pinout_set_addr_ddr(enum pin_direction direction);

// Write a value to the data bus.
// Requires that the data bus DDR is set to output.
void pinout_write_data(uint8_t data);

// Read a value from the data bus.
// Requires that the data bus DDR is set to input.
uint8_t pinout_read_data(void);

// Write a value to the address bus.
// Requires that the address bus DDR is set to output.
void pinout_write_addr(uint16_t addr);

// Read a value from the address bus.
// Requires that the address bus DDR is set to input.
uint16_t pinout_read_addr(void);

#endif
