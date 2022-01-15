#include "buffer.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

void buffer_init(struct buffer* buf) {
    buf->data = NULL;
    buf->size = 0;
    buf->capacity = 0;
}

void buffer_deinit(struct buffer* buf) {
    free(buf->data);
}

void buffer_ensure_total_capacity(struct buffer* buf, size_t new_capacity) {
    size_t better_capacity = buf->capacity;
    if (better_capacity >= new_capacity)
        return;

    do {
        better_capacity += better_capacity / 2 + 8;
    } while (better_capacity < new_capacity);

    void* new_data = realloc(buf->data, better_capacity);
    assert(new_data);
    buf->data = new_data;
    buf->capacity = new_capacity;
}

void buffer_ensure_unused_capacity(struct buffer* buf, size_t unused_capacity) {
    return buffer_ensure_total_capacity(buf, buf->capacity + unused_capacity);
}

void buffer_push_data(struct buffer* buf, size_t len, const void* data) {
    buffer_ensure_unused_capacity(buf, len);
    memcpy(buffer_end(buf), data, len);
    buf->size += len;
}
