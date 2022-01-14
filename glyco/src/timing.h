#ifndef GLYCO_SRC_TIMING_H
#define GLYCO_SRC_TIMING_H

#include <util/delay.h>

// General delay to wait between when a pin is written and when the result has propagated.
// Delay value was found by experimentation - the minimum delay which
// produced correct results was about 500ns, double that for safety.
#define TIMING_PIN_DELAY_US (1)

// Maximim flash write delay (from spec).
#define TIMING_FLASH_WRITE_DELAY_US (20)

// Maximim flash sector erase delay (from spec).
#define TIMING_FLASH_ERASE_SECTOR_MS (25)

// Maximum flash chip erase delay (from spec).
#define TIMING_FLASH_ERASE_CHIP_MS (100)

#define timing_delay() _delay_us(TIMING_PIN_DELAY_US)

#define timing_flash_write_delay() _delay_us(TIMING_FLASH_WRITE_DELAY_US)

#define timing_flash_erase_sector_delay() _delay_ms(TIMING_FLASH_ERASE_SECTOR_MS)

#define timing_flash_erase_chip_delay() _delay_ms(TIMING_FLASH_ERASE_CHIP_MS)

#endif
