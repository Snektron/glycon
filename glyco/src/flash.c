#include "flash.h"
#include "pinout.h"
#include "bus.h"
#include "timing.h"

#include <stddef.h>

#define FLASH_SOFTWARE_ID_MFG_ADDR (0x0000)
#define FLASH_SOFTWARE_ID_DEV_ADDR (0x0001)

// General delay between pin writes and reads and stuff.
#define delay() _delay_us(1)

// Maximum delay after writing, from the ROM spec.
#define write_delay() _delay_us(20)

static void flash_cmd(uint16_t addr, uint8_t data) {
    bus_write(addr, data);
    PINOUT_FLASH_WE_PORT &= ~PINOUT_FLASH_WE_MASK;
    timing_delay();
    PINOUT_FLASH_WE_PORT |= PINOUT_FLASH_WE_MASK;
    timing_delay();
}

// Prepare the system for writing a flash command
static void flash_begin_cmd(void) {
    bus_set_mode(BUS_MODE_WRITE_MEM);
}

void flash_byte_program(uint16_t address, uint8_t data) {
    if ((address & 0x8000) != 0) // Don't attempt to write to RAM.
        return;
    flash_begin_cmd();
    flash_cmd(0x5555, 0xAA);
    flash_cmd(0x2AAA, 0x55);
    flash_cmd(0x5555, 0xA0);
    flash_cmd(address, data);
    timing_flash_write_delay();
}

static void flash_enter_software_id_mode(void) {
    flash_begin_cmd();
    flash_cmd(0x5555, 0xAA);
    flash_cmd(0x2AAA, 0x55);
    flash_cmd(0x5555, 0x90);
}

static void flash_exit_software_id_mode(void) {
    flash_begin_cmd();
    flash_cmd(0x5555, 0xAA);
    flash_cmd(0x2AAA, 0x55);
    flash_cmd(0x5555, 0xF0);
}

void flash_get_software_id(uint8_t* mfg, uint8_t* dev) {
    flash_enter_software_id_mode();
    bus_set_mode(BUS_MODE_READ_MEM);
    *mfg = bus_read(FLASH_SOFTWARE_ID_MFG_ADDR);
    *dev = bus_read(FLASH_SOFTWARE_ID_DEV_ADDR);
    flash_exit_software_id_mode();
}
