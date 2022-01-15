#ifndef GLYDB_SRC_BUFFER_H
#define GLYDB_SRC_BUFFER_H

#include <stddef.h>

// Generic resizable buffer that can be used as array list.
struct buffer {
    void* data;
    size_t size;
    size_t capacity;
};

void buffer_init(struct buffer* buf);
void buffer_deinit(struct buffer* buf);

void buffer_ensure_total_capacity(struct buffer* buf, size_t new_capacity);
void buffer_ensure_unused_capacity(struct buffer* buf, size_t unused_capacity);

void buffer_push_data(struct buffer* buf, size_t len, const void* data);

static inline void* buffer_end(struct buffer* buf) {
    return &buf->data[buf->size];
}

#endif
