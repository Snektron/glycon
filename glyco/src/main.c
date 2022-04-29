#include "pinout.h"
#include "serial.h"
#include "flash.h"
#include "bus.h"

#include "common/glycon.h"
#include "common/binary_debug_protocol.h"

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#include <avr/interrupt.h>
#include <util/delay.h>

// Read an address from a BDBP data buffer.
gly_addr_t pkt_read_addr(uint8_t** data_ptr) {
    uint8_t* data = *data_ptr;
    gly_addr_t a = *data++;
    gly_addr_t b = *data++;
    gly_addr_t c = *data++;
    *data_ptr = data;
    return (c << 16) | (b << 8) | a;
}

// Try to acquire the Z80's bus. If that fails, return false,
// and return an error status.
bool acquire_bus_or_fail() {
    enum bus_acquire_status status = bus_acquire();
    switch (status) {
        case BUS_ACQUIRE_SUCCESS:
            return true;
        case BUS_ACQUIRE_TIMEOUT:
            serial_write_byte(BDBP_STATUS_BUS_ACQUIRE_TIMEOUT);
            serial_write_byte(0);
            return false;
        default:
            __builtin_unreachable();
    }
}

// Handle CMD_WRITE: Write some data to memory.
void cmd_write(uint8_t* data, uint8_t* data_end) {
    if (!acquire_bus_or_fail())
        return;

    gly_addr_t address = pkt_read_addr(&data);
    bus_set_mode(BUS_MODE_WRITE_MEM);
    while (data != data_end) {
        bus_write(address++, *data++);
        bus_pulse_ram_write();
    }
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

// Handle CMD_READ: Read some data from ram- or rom.
void cmd_read(uint8_t* data, uint8_t* data_end) {
    if (!acquire_bus_or_fail())
        return;
    gly_addr_t address = pkt_read_addr(&data);
    uint8_t amt = *data++;

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(amt);

    bus_set_mode(BUS_MODE_READ_MEM);
    for (uint8_t i = 0; i < amt; ++i) {
        serial_write_byte(bus_read(address + i));
    }
    bus_release();
}

// Handle CMD_FLASH: Write some data to flash storage.
void cmd_flash(uint8_t* data, uint8_t* data_end) {
    if (!acquire_bus_or_fail())
        return;

    gly_addr_t address = pkt_read_addr(&data);
    while (data != data_end) {
        flash_byte_program(address++, *data++);
    }
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

// Handle CMD_FLASH_ID: Returns the flash's manufacterer and device identifiers.
void cmd_flash_id() {
    if (!acquire_bus_or_fail())
        return;
    uint8_t mfg, dev;
    flash_get_software_id(&mfg, &dev);
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(2);
    serial_write_byte(mfg);
    serial_write_byte(dev);
}

// Handle CMD_ERASE_SECTOR: Erases a single flash sector.
void cmd_erase_sector(uint8_t* data, uint8_t* data_end) {
    if (!acquire_bus_or_fail())
        return;

    gly_addr_t address = pkt_read_addr(&data);
    flash_erase_sector(address);
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

// Handle CMD_ERASE_CHIP: Erases the entire flash chip.
void cmd_erase_chip() {
    if (!acquire_bus_or_fail())
        return;
    flash_erase_chip();
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

int main(void) {
    PINOUT_LED_DDR |= PINOUT_LED_MASK;

    PINOUT_BUSREQ_DDR |= PINOUT_BUSREQ_MASK;
    PINOUT_BUSREQ_PORT &= ~PINOUT_BUSREQ_MASK;

    PINOUT_BUSACK_DDR &= ~PINOUT_BUSACK_MASK;
    PINOUT_BUSACK_PORT &= ~PINOUT_BUSACK_MASK;

    PINOUT_RESET_DDR &= ~PINOUT_RESET_DDR;
    PINOUT_RESET_PORT &= ~PINOUT_RESET_MASK;

    PINOUT_M1_DDR &= ~PINOUT_M1_DDR;
    PINOUT_M1_PORT &= ~PINOUT_M1_MASK;

    PINOUT_IOREQ_DDR &= ~PINOUT_IOREQ_DDR;
    PINOUT_IOREQ_PORT &= ~PINOUT_IOREQ_MASK;

    serial_init();
    sei();

    while (1) {
        // Use led to indicate processing.
        PINOUT_LED_PORT |= PINOUT_LED_MASK;
        serial_wait_for_data();
        PINOUT_LED_PORT &= ~PINOUT_LED_MASK;
        uint8_t cmd = serial_read_u8();
        uint8_t data_len = serial_poll_u8();

        uint8_t msg_data[BDBP_MAX_DATA_LENGTH];
        for (size_t i = 0; i < data_len; ++i) {
            msg_data[i] = serial_poll_u8();
        }

        switch (cmd) {
            case BDBP_CMD_PING:
                serial_write_byte(BDBP_STATUS_SUCCESS);
                serial_write_byte(0);
                break;
            case BDBP_CMD_WRITE:
                cmd_write(msg_data, msg_data + data_len);
                break;
            case BDBP_CMD_READ:
                cmd_read(msg_data, msg_data + data_len);
                break;
            case BDBP_CMD_WRITE_FLASH:
                cmd_flash(msg_data, msg_data + data_len);
                break;
            case BDBP_CMD_FLASH_ID:
                cmd_flash_id();
                break;
            case BDBP_CMD_ERASE_SECTOR:
                cmd_erase_sector(msg_data, msg_data + data_len);
                break;
            case BDBP_CMD_ERASE_CHIP:
                cmd_erase_chip();
                break;
            default:
                // Delete any data bytes that follow
                for (size_t i = 0; i < data_len; ++i) {
                    serial_poll_u8();
                }
                serial_write_byte(BDBP_STATUS_UNKNOWN_CMD);
                serial_write_byte(0);
                break;
        }
    }

    return 0;
}
