#include "pinout.h"
#include "serial.h"

#include "bdbp/binary_debug_protocol.h"

#include <stdint.h>

#include <avr/interrupt.h>
#include <util/delay.h>

int main(void) {
    PINOUT_LED_DDR |= PINOUT_LED_MASK;

    serial_init();
    sei();

    pinout_set_data_ddr(PIN_OUTPUT);

    while (1) {
        serial_wait_for_data();
        PINOUT_LED_PORT ^= PINOUT_LED_MASK;
        uint8_t cmd = serial_read_byte();
        switch (cmd) {
            case BDBP_CMD_PING:
                serial_write_byte(BDBP_STATUS_SUCCESS);
                break;
            default:
                serial_write_byte(BDBP_STATUS_UNKNOWN_CMD);
                break;
        }
    }

    return 0;
}
