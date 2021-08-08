#include "pinout.h"
#include "serial.h"

#include <avr/interrupt.h>
#include <util/delay.h>

int main() {
    LED_DDR |= LED_MASK;
    LED_PORT |= LED_MASK;

    serial_init();
    sei();

    while (1) {
        int c = serial_read_byte();
        if (c > 0) {
            LED_PORT ^= LED_MASK;
            serial_write_byte(c);
        }
    }

    return 0;
}
