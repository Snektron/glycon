#ifndef _GLYCO_SERIAL_H
#define _GLYCO_SERIAL_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the serial hardware.
void serial_init();

// Return the number of bytes available in the receive buffer.
uint16_t serial_avail();

// Block until data becomes available in the receive buffer. If
// data is not currently available, this function will sleep the
// cpu until data is available.
void serial_wait_for_data();

// Poll until data becomes available in the receive buffer. In contrast
// to `serial_wait_for_data`, this function will not sleep the cpu.
void serial_poll_for_data();

// Return the next byte in the serial receive buffer.
// If no data is available, returns -1.
int serial_read_u8();

// Block until the next byte is available.
uint8_t serial_poll_u8();

// Write one byte to serial. Blocks until the byte is written.
void serial_write_byte(uint8_t value);

#endif
