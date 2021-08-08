#ifndef _GLYCON_COPROCESSOR_PINOUT_H
#define _GLYCON_COPROCESSOR_PINOUT_H

#include <avr/io.h>

#define LED_DDR DDRB
#define LED_PORT PORTB
#define LED_MASK (1 << PB7)

#endif
