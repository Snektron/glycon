#include "pinout.h"
#include "serial.h"

#include <avr/interrupt.h>
#include <util/delay.h>

int main() {
    PINOUT_LED_DDR |= PINOUT_LED_MASK;

    serial_init();
    sei();

    pinout_set_data_ddr(0);

    int a = 0;
    while (1) {
        pinout_write_data(-a);
        a ^= 1;
        PINOUT_LED_PORT &= ~PINOUT_LED_MASK;
        PINOUT_LED_PORT |= -a & PINOUT_LED_MASK;
        _delay_ms(5000);
    }

    return 0;
}
