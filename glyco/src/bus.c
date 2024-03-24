#include "bus.h"

#include <util/delay.h>

// Timeout to wait until considering the bus acquire a failure.
// This value is just a rough limit, the pin status will be polled
// every ms until this amount of polls have been done.
#define BUS_ACQUIRE_TIMEOUT_US (1000)

enum bus_acquire_status bus_acquire(void) {
    if ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) == 0) {
        return BUS_ACQUIRE_ACQUIRED;
    }
    // Acquire bus from the Z80: pull the busreq pin low,
    // and loop until the busack pin is low.
    PINOUT_BUSREQ_PORT |= PINOUT_BUSREQ_MASK;
    int delay = 0;
    while ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) != 0 && delay < BUS_ACQUIRE_TIMEOUT_US) {
        _delay_us(10);
        ++delay;
    }
    if ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) != 0) {
        PINOUT_BUSREQ_PORT &= ~PINOUT_BUSREQ_MASK;
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
    PINOUT_RAM_WE_PORT |= PINOUT_RAM_WE_MASK;

    PINOUT_MEM_OE_DDR &= ~PINOUT_MEM_OE_MASK;
    bus_enable_mem_output(false);

    PINOUT_FLASH_WE_DDR &= ~PINOUT_FLASH_WE_MASK;
    PINOUT_FLASH_WE_PORT &= PINOUT_FLASH_WE_MASK;

    pinout_set_addr_ddr(PIN_INPUT);

    // Release Z80 bus.
    // TODO: Should
    PINOUT_BUSREQ_PORT &= ~PINOUT_BUSREQ_MASK;
    while ((PINOUT_BUSACK_PIN & PINOUT_BUSACK_MASK) == 0)
        continue;
}
