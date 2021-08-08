#ifndef _GLYCON_COPROCESSOR_SERIAL_H
#define _GLYCON_COPROCESSOR_SERIAL_H

#include <stdint.h>
#include <stdbool.h>

// Initialize the serial hardware.
void serial_init();

// Return the number of bytes available in the receive buffer.
uint16_t serial_avail();

// Block until data becomes available in the receive buffer.
void serial_wait_for_data();

// Return the next byte in the serial receive buffer.
// If no data is available, returns -1.
int serial_read_byte();

void serial_write_byte_sync(uint8_t value);

// Write one byte to serial. Blocks until the byte is written.
void serial_write_byte(uint8_t value);

#endif
