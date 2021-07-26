#include <avr/io.h>
#include <util/delay.h>

#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_MASK (1 << PB7)

int main() {
    LED_DDR |= LED_MASK;
    LED_PORT |= LED_MASK;

    while (1) {
        LED_PORT |= LED_MASK;
        _delay_ms(100);
        LED_PORT &= ~LED_MASK;
        _delay_ms(100);
    }

    return 0;
}
