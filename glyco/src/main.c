#include "pinout.h"
#include "serial.h"
#include "flash.h"
#include "bus.h"

#include "bdbp/binary_debug_protocol.h"

#include <stdint.h>
#include <stddef.h>

#include <avr/interrupt.h>
#include <util/delay.h>

void cmd_write(uint8_t data_len) {
    uint8_t address_hi = serial_poll_byte();
    uint8_t address_lo = serial_poll_byte();
    data_len -= 2;
    uint16_t address = (address_hi << 8) | address_lo;

    bus_acquire();
    bus_set_mode(BUS_MODE_WRITE_MEM);
    for (uint8_t i = 0; i < data_len; ++i) {
        uint8_t data = serial_poll_byte();
        bus_write(address + i, data);
        bus_pulse_ram_write();
    }

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

void cmd_read() {
    uint8_t address_hi = serial_poll_byte();
    uint8_t address_lo = serial_poll_byte();
    uint8_t amt = serial_poll_byte();
    uint16_t address = (address_hi << 8) | address_lo;

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(amt);

    bus_acquire();
    bus_set_mode(BUS_MODE_READ_MEM);
    for (uint8_t i = 0; i < amt; ++i) {
        uint8_t data = bus_read(address + i);
        serial_write_byte(data);
    }
    bus_release();
}

void cmd_flash(uint8_t data_len) {
    uint8_t address_hi = serial_poll_byte();
    uint8_t address_lo = serial_poll_byte();
    data_len -= 2;
    uint8_t data = serial_poll_byte();
    uint16_t address = (address_hi << 8) | address_lo;

    bus_acquire();
    flash_byte_program(address, data);
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

void cmd_flash_id() {
    bus_acquire();
    uint8_t mfg, dev;
    flash_get_software_id(&mfg, &dev);
    bus_release();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(2);
    serial_write_byte(mfg);
    serial_write_byte(dev);
}

int main(void) {
    PINOUT_LED_DDR |= PINOUT_LED_MASK;

    bus_init();
    serial_init();
    sei();

    while (1) {
        serial_wait_for_data();
        PINOUT_LED_PORT ^= PINOUT_LED_MASK;
        uint8_t cmd = serial_read_byte();
        uint8_t data_len = serial_poll_byte();
        switch (cmd) {
            case BDBP_CMD_PING:
                serial_write_byte(BDBP_STATUS_SUCCESS);
                serial_write_byte(0);
                break;
            case BDBP_CMD_WRITE:
                cmd_write(data_len);
                break;
            case BDBP_CMD_READ:
                cmd_read();
                break;
            case BDBP_CMD_WRITE_FLASH:
                cmd_flash(data_len);
                break;
            case BDBP_CMD_FLASH_ID:
                cmd_flash_id();
                break;
            default:
                // Delete any data bytes that follow
                for (size_t i = 0; i < data_len; ++i) {
                    serial_poll_byte();
                }
                serial_write_byte(BDBP_STATUS_UNKNOWN_CMD);
                serial_write_byte(0);
                break;
        }
    }

    return 0;
}
