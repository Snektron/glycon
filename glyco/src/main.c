#include "pinout.h"
#include "serial.h"

#include "bdbp/binary_debug_protocol.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <util/delay.h>

void cmd_write(void) {
    serial_poll_for_data();
    uint8_t address_hi = serial_read_byte();
    serial_poll_for_data();
    uint8_t address_lo = serial_read_byte();
    serial_poll_for_data();
    uint8_t data = serial_read_byte();
    uint8_t address = address_hi << 8 | address_lo;

    pinout_set_data_ddr(PIN_OUTPUT);
    pinout_write_addr(address);
    pinout_write_data(data);
    _delay_us(1);
    PINOUT_RAM_WE_PORT &= ~PINOUT_RAM_WE_MASK;
    _delay_us(1);
    PINOUT_RAM_WE_PORT |= PINOUT_RAM_WE_MASK;
    _delay_us(1);
    pinout_set_data_ddr(PIN_INPUT);
    data = pinout_read_data();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(data);
}

void cmd_read(void) {
    serial_poll_for_data();
    uint8_t address_hi = serial_read_byte();
    serial_poll_for_data();
    uint8_t address_lo = serial_read_byte();
    uint8_t address = address_hi << 8 | address_lo;

    pinout_write_addr(address);
    _delay_us(1);
    uint8_t data = pinout_read_data();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(data);
}

int main(void) {
    PINOUT_LED_DDR |= PINOUT_LED_MASK;
    PINOUT_RAM_WE_DDR |= PINOUT_RAM_WE_MASK;
    PINOUT_RAM_WE_PORT |= PINOUT_RAM_WE_MASK;

    serial_init();
    sei();

    pinout_set_addr_ddr(PIN_OUTPUT);
    pinout_set_data_ddr(PIN_INPUT);

    while (1) {
        serial_wait_for_data();
        PINOUT_LED_PORT ^= PINOUT_LED_MASK;
        uint8_t cmd = serial_read_byte();
        switch (cmd) {
            case BDBP_CMD_PING:
                serial_write_byte(BDBP_STATUS_SUCCESS);
                break;
            case BDBP_CMD_WRITE:
                cmd_write();
                break;
            case BDBP_CMD_READ:
                cmd_read();
                break;
            default:
                serial_write_byte(BDBP_STATUS_UNKNOWN_CMD);
                break;
        }
    }

    return 0;
}
