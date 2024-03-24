#ifndef GLYDB_SRC_BUFFER_H
#define GLYDB_SRC_BUFFER_H

#include <stddef.h>

// Generic resizable buffer that can be used as array list.
struct buffer {
    // A pointer to the buffer's data.
    void* data;
    // The number of valid bytes in `data`.
    size_t size;
    // The total capacity of the buffer in bytes.
    size_t capacity;
};

// Initialize `buf` to a new, empty buffer.
void buffer_init(struct buffer* buf);
// Deinitialize `buf`, and free its internal resources.
void buffer_deinit(struct buffer* buf);

// Ensure that the buffer holds capacity for at least `new_capacity` bytes.
void buffer_ensure_total_capacity(struct buffer* buf, size_t new_capacity);
// Ensure that the buffer holds capacity for at least another `unused_capacity` bytes.
void buffer_ensure_unused_capacity(struct buffer* buf, size_t unused_capacity);

// Push some bytes into the buffer.
// Invalidates any pointers into `data`.
void buffer_push_data(struct buffer* buf, size_t len, const void* data);

// Return a pointer to the end of the current buffer.
static inline void* buffer_end(struct buffer* buf) {
    return buf->data + buf->size;
}

#endif
