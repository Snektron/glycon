#ifndef _GLYCO_BUS_H
#define _GLYCO_BUS_H

#include "pinout.h"
#include "timing.h"

#include <stdint.h>

// Utility enum used to quickly prepare the bus for a certain operation.
enum bus_mode {
    // Prepare the bus for a memory device (ram or flash) write operation.
    BUS_MODE_WRITE_MEM,

    // Prepare the bus for a memory device (ram or flash) read operation.
    BUS_MODE_READ_MEM,
};

void bus_init(void);
void bus_acquire(void);
void bus_release(void);

static inline void bus_enable_mem_output(bool enable) {
    if (enable) {
        PINOUT_MEM_OE_PORT &= ~PINOUT_MEM_OE_MASK;
    } else {
        PINOUT_MEM_OE_PORT |= PINOUT_MEM_OE_MASK;
    }
}

// Set address DDR, data DDR, and mem output DDR based on `mode`.
// Requires bus acquired.
// Note: Address bus is always in output mode when the bus is acquired.
static inline void bus_set_mode(enum bus_mode mode) {
    switch (mode) {
        case BUS_MODE_WRITE_MEM:
            bus_enable_mem_output(false);
            pinout_set_data_ddr(PIN_OUTPUT);
            break;
        case BUS_MODE_READ_MEM:
            bus_enable_mem_output(true);
            pinout_set_data_ddr(PIN_INPUT);
            pinout_write_data(0);
            break;
    }
}

// Write a value to the bus at a particular address. Includes delay.
// Requires BUS_MODE_WRITE_MEM.
static inline void bus_write(uint16_t addr, uint8_t data) {
    pinout_write_addr(addr);
    pinout_write_data(data);
    timing_delay();
}

// Read a value from the bus at a particular address.
// Requires BUS_MODE_READ_MEM.
static inline uint8_t bus_read(uint16_t addr) {
    pinout_write_addr(addr);
    timing_delay();
    return pinout_read_data();
}

// Momentarily pull the RAM write enable pin low, which writes the data currently
// on the data bus to address if the address' msb is high.
// Requires bus acquired.
static inline void bus_pulse_ram_write(void) {
    PINOUT_RAM_WE_PORT &= ~PINOUT_RAM_WE_MASK;
    timing_delay();
    PINOUT_RAM_WE_PORT |= PINOUT_RAM_WE_MASK;
    timing_delay();
}

// Momentarily pull the FLASH write enable pin high, which writes the data currently
// on the data bus to address if the address' msb is low.
// Requires bus acquired.
static inline void bus_pulse_flash_write(void) {
    PINOUT_FLASH_WE_PORT |= PINOUT_FLASH_WE_MASK;
    timing_delay();
    PINOUT_FLASH_WE_PORT &= ~PINOUT_FLASH_WE_MASK;
    timing_delay();
}

#endif
