#include "pinout.h"
#include "serial.h"

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
        pinout_write_data(serial_read_byte());
        PINOUT_LED_PORT ^= PINOUT_LED_MASK;
    }

    return 0;
}
