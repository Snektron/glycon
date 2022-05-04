#include "serial.h"

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <util/setbaud.h>

// The amount of bytes that can be received at once.
// Note: must be a power of 2.
// Note: must be half range of index type (u16).
#define SERIAL_BUFFER_SIZE 512
#define SERIAL_BUFFER_MASK (SERIAL_BUFFER_SIZE - 1)

struct ring_buffer {
    uint8_t data[SERIAL_BUFFER_SIZE];
    uint16_t read;
    uint16_t write;
};

#define ring_buffer_write(ring, value) ((ring)->data[(ring)->write++ & SERIAL_BUFFER_MASK] = (value))
#define ring_buffer_read(ring) ((ring)->data[(ring)->read++ & SERIAL_BUFFER_MASK])
#define ring_buffer_size(ring) ((ring)->write - (ring)->read)
#define ring_buffer_free(ring) (SERIAL_BUFFER_SIZE - ring_buffer_size(ring))
#define ring_buffer_is_full(ring) (ring_buffer_size(ring) == SERIAL_BUFFER_SIZE)
#define ring_buffer_is_empty(ring) (ring_buffer_size(ring) == 0)

volatile struct ring_buffer rx_buffer;

void serial_init() {
    // Set baud rate.
    UBRR0H = UBRRH_VALUE;
    UBRR0L = UBRRL_VALUE;

    // Clear status and enable baud doubler if needed.
    #if USE_2X
        UCSR0A = 1 << U2X0;
    #else
        UCSR0A = 0;
    #endif

    // Enable receive/tranfer, and receive interrupts.
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

    // 8-bit data, no parity, 1-bit stop.
    UCSR0C = (1 << UCSZ01) | (1 << UCSZ00);

    // Clear receive buffers.
    rx_buffer.read = rx_buffer.write = 0;
}

uint16_t serial_avail() {
    return ring_buffer_size(&rx_buffer);
}

void serial_wait_for_data() {
    if (!ring_buffer_is_empty(&rx_buffer))
        return;

    // Note: any other sleep mode is not woken up by the USARTs
    set_sleep_mode(SLEEP_MODE_IDLE);
    cli();
    while (ring_buffer_is_empty(&rx_buffer)) {
        sleep_enable();
        sei();
        sleep_cpu();
        sleep_disable();
        cli();
    }
    sei();
}

void serial_poll_for_data() {
    while (ring_buffer_is_empty(&rx_buffer))
        continue;
}

int serial_read_u8() {
    if (ring_buffer_is_empty(&rx_buffer))
        return -1;
    return ring_buffer_read(&rx_buffer);
}

uint8_t serial_poll_u8() {
    serial_poll_for_data();
    return ring_buffer_read(&rx_buffer);
}

void serial_write_u8(uint8_t value) {
    loop_until_bit_is_set(UCSR0A, UDRE0);
    UDR0 = value;
}

ISR(USART0_RX_vect) {
    uint8_t data = UDR0;

    // If full, skip
    if (!ring_buffer_is_full(&rx_buffer)) {
        ring_buffer_write(&rx_buffer, data);
    }
}
