#include "pinout.h"
#include "serial.h"

#include <avr/interrupt.h>
#include <util/delay.h>

int main() {
    PINOUT_LED_DDR |= PINOUT_LED_MASK;

    serial_init();
    sei();

    pinout_set_data_ddr(PIN_OUTPUT);

    int a = 0;
    while (1) {
        a ^= 1;
        pinout_write_data(-a);
        PINOUT_LED_PORT &= ~PINOUT_LED_MASK;
        PINOUT_LED_PORT |= -a & PINOUT_LED_MASK;
        _delay_ms(100);
    }

    return 0;
}
