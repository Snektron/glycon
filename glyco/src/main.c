#include "pinout.h"
#include "serial.h"

#include "bdbp/binary_debug_protocol.h"

#include <stdint.h>
#include <stddef.h>

#include <avr/interrupt.h>
#include <util/delay.h>

// Delay value was found by experimentation - the minimum delay which
// produced correct results was about 500ns, double that for safety.
#define delay() _delay_us(1);

void cmd_write(uint8_t data_len) {
    uint8_t address_hi = serial_poll_byte();
    uint8_t address_lo = serial_poll_byte();
    uint8_t data = serial_poll_byte();
    uint16_t address = (address_hi << 8) | address_lo;

    pinout_set_data_ddr(PIN_OUTPUT);
    pinout_write_addr(address);
    pinout_write_data(data);
    delay();
    PINOUT_RAM_WE_PORT &= ~PINOUT_RAM_WE_MASK;
    delay();
    PINOUT_RAM_WE_PORT |= PINOUT_RAM_WE_MASK;
    delay();
    pinout_set_data_ddr(PIN_INPUT);
    pinout_write_data(0);

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(0);
}

void cmd_read(uint8_t data_len) {
    uint8_t address_hi = serial_poll_byte();
    uint8_t address_lo = serial_poll_byte();
    uint8_t amt = serial_poll_byte();
    (void) amt;
    uint16_t address = (address_hi << 8) | address_lo;
    pinout_write_addr(address);
    delay();
    uint8_t data = pinout_read_data();

    serial_write_byte(BDBP_STATUS_SUCCESS);
    serial_write_byte(1);
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
                cmd_read(data_len);
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
