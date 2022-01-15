#include "bus.h"

void bus_acquire(void) {
    // TODO: Acquire bus from Z80.

    pinout_write_data(0);
    pinout_set_data_ddr(PIN_INPUT);

    // Set address to 0 prior to enabling ram WE so that we don't accidently write to it.
    pinout_write_addr(0);
    pinout_set_addr_ddr(PIN_OUTPUT);

    PINOUT_RAM_WE_PORT |= PINOUT_RAM_WE_MASK;
    PINOUT_RAM_WE_DDR |= PINOUT_RAM_WE_MASK;

    bus_enable_mem_output(true);
    PINOUT_MEM_OE_DDR |= PINOUT_MEM_OE_MASK;

    PINOUT_FLASH_WE_PORT &= ~PINOUT_FLASH_WE_MASK;
    PINOUT_FLASH_WE_DDR |= PINOUT_FLASH_WE_MASK;
}

void bus_release(void) {
    pinout_write_data(0);
    pinout_set_data_ddr(PIN_INPUT);

    // Deselect ram chip before releasing ram WE pin.
    pinout_write_addr(0);
    PINOUT_RAM_WE_DDR &= ~PINOUT_RAM_WE_MASK;
    PINOUT_RAM_WE_PORT &= ~PINOUT_RAM_WE_MASK;

    PINOUT_MEM_OE_DDR &= ~PINOUT_MEM_OE_MASK;
    bus_enable_mem_output(false);

    PINOUT_FLASH_WE_DDR &= ~PINOUT_FLASH_WE_MASK;
    PINOUT_FLASH_WE_PORT &= PINOUT_FLASH_WE_MASK;

    // TODO: Release Z80 bus.
}
