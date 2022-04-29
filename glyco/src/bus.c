#include "bus.h"

#include <util/delay.h>

enum bus_acquire_status bus_acquire(void) {
    // Acquire bus from the Z80: pull the busreq pin low,
    // and loop until the busack pin is low.
    PINOUT_BUSREQ_PORT |= PINOUT_BUSREQ_MASK;
    int delay = 0;
    while ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) != 0 && delay < BUS_ACQUIRE_TIMEOUT_MS) {
        _delay_ms(1);
        ++delay;
    }
    if ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) != 0) {
        return BUS_ACQUIRE_TIMEOUT;
    }

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

    return BUS_ACQUIRE_SUCCESS;
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

    // Release Z80 bus.
    PINOUT_BUSREQ_PORT &= ~PINOUT_BUSREQ_MASK;
    while ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) == 0)
        continue;
}
