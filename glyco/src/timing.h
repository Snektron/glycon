#ifndef GLYCO_SRC_TIMING_H
#define GLYCO_SRC_TIMING_H

#include <util/delay.h>

// General delay to wait between when a pin is written and when the result has propagated.
// Delay value was found by experimentation - the minimum delay which
// produced correct results was about 500ns, double that for safety.
#define TIMING_PIN_DELAY_US (1)

// Maximim flash write delay (from spec).
#define TIMING_FLASH_WRITE_DELAY_US (20)

#define timing_delay() _delay_us(TIMING_PIN_DELAY_US)

#define timing_flash_write_delay() _delay_us(TIMING_FLASH_WRITE_DELAY_US)

#endif
