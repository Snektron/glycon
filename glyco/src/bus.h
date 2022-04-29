#ifndef _GLYCO_BUS_H
#define _GLYCO_BUS_H

#include "common/glycon.h"

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

// Acquire bus ownership from the Z80 cpu. This puts the Z80 on hold, and initializes
// all arduino pins to their proper state. It is invalid to write to any of the
// arduinos' pins or call `bus_*` functions before this is called.
void bus_acquire(void);

// Release the bus ownership, and let the Z80 continue processing. This disables all of
// the relevant output pins of the arduino so that it does not interfere with the Z80.
// After this, it is invalid to write to arduino pins or to call any bus_* function until
// the bus is acquired again with `bus_acquire`.
void bus_release(void);

// Set the memory chip's output-enable status. When the memory chip's output is enabled,
// it places the data at the location given by the address pins on the data bus, and so
// this is required to read data from the memory chip.
// Note, the memory chip is only enabled when the address bus contains an address
// higher than or equal to 0x8000. It is valid to enable the memory chip's output
// and read from a different device with address lower than 0x8000. In particular,
// flash and memory can be read simultaneously while this setting is enabled.
// Note, that the memory chip's output should be disabled when reading from any other
// device to prevent the data signals from interfering.
static inline void bus_enable_mem_output(bool enable) {
    if (enable) {
        PINOUT_MEM_OE_PORT &= ~PINOUT_MEM_OE_MASK;
    } else {
        PINOUT_MEM_OE_PORT |= PINOUT_MEM_OE_MASK;
    }
}

// Set address DDR, data DDR, and mem output DDR based on `mode`, to quickly
// configure the bus for a particular operation.
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
static inline void bus_write(gly_addr_t addr, uint8_t data) {
    pinout_write_addr(addr);
    pinout_write_data(data);
    timing_delay();
}

// Read a value from the bus at a particular address.
// Requires BUS_MODE_READ_MEM.
static inline uint8_t bus_read(gly_addr_t addr) {
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
