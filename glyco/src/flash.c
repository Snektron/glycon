#include "flash.h"
#include "pinout.h"

#include <stddef.h>
#include <util/delay.h>

#define FLASH_SOFTWARE_ID_MFG_ADDR (0x0000)
#define FLASH_SOFTWARE_ID_DEV_ADDR (0x0001)

// General delay between pin writes and reads and stuff.
#define delay() _delay_us(1)

// Maximum delay after writing, from the ROM spec.
#define write_delay() _delay_us(20)

static void flash_pulse_write(void) {
    PINOUT_FLASH_WE_PORT &= ~PINOUT_FLASH_WE_MASK;
    delay();
    PINOUT_FLASH_WE_PORT |= PINOUT_FLASH_WE_MASK;
    delay();
}

static void flash_cmd(uint16_t addr, uint8_t data) {
    pinout_write_addr(addr);
    pinout_write_data(data);
    delay();
    flash_pulse_write();
}

// Prepare the system for writing a flash command
static void flash_begin_cmd(void) {
    pinout_set_data_ddr(PIN_OUTPUT);
    pinout_enable_mem_output(false);
}

// Clean up the system state after writing a flash command
static void flash_end_cmd(void) {
    pinout_enable_mem_output(true);
    pinout_set_data_ddr(PIN_INPUT);
    pinout_write_data(0);
    delay();
}

void flash_init() {
    // Neutral state has write disabled (high).
    PINOUT_FLASH_WE_DDR |= PINOUT_FLASH_WE_MASK;
    PINOUT_FLASH_WE_PORT |= PINOUT_FLASH_WE_MASK;
}

void flash_byte_program(uint16_t address, uint8_t data) {
    flash_begin_cmd();
    flash_cmd(0x5555, 0xAA);
    flash_cmd(0x2AAA, 0x55);
    flash_cmd(0x5555, 0xA0);
    flash_cmd(address, data);
    write_delay();
    flash_end_cmd();
}

static void flash_enter_software_id_mode(void) {
    flash_begin_cmd();
    flash_cmd(0x5555, 0xAA);
    flash_cmd(0x2AAA, 0x55);
    flash_cmd(0x5555, 0x90);
    flash_end_cmd();
}

static void flash_exit_software_id_mode(void) {
    flash_begin_cmd();
    flash_cmd(0x5555, 0xAA);
    flash_cmd(0x2AAA, 0x55);
    flash_cmd(0x5555, 0xF0);
    flash_end_cmd();
}

void flash_get_software_id(uint8_t* mfg, uint8_t* dev) {
    flash_enter_software_id_mode();
    // data bus is still in input mode.
    pinout_write_addr(FLASH_SOFTWARE_ID_MFG_ADDR);
    delay();
    *mfg = pinout_read_data();
    pinout_write_addr(FLASH_SOFTWARE_ID_DEV_ADDR);
    delay();
    *dev = pinout_read_data();
    flash_exit_software_id_mode();
}
