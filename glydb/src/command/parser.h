#ifndef _GLYDB_COMMAND_PARSER_H
#define _GLYDB_COMMAND_PARSER_H

#include <stddef.h>
#include <stdbool.h>

struct parser {
    const char* input;
    size_t length;
    size_t offset;
};


inline int parser_is_at_end(struct parser* p) {
    return p->offset >= p->length;
}

inline int parser_peek(struct parser* p) {
    if (parser_is_at_end(p))
        return -1;
    return p->input[p->offset];
}

inline int parser_consume(struct parser* p) {
    if (parser_is_at_end(p))
        return -1;
    return p->input[p->offset++];
}

inline bool parser_eat(struct parser* p, char expected) {
    int c = parser_peek(p);
    if (c == expected) {
        ++p->offset;
        return true;
    }
    return false;
}

inline const char* parser_remaining(struct parser* p) {
    return &p->input[p->offset];
}

inline size_t parser_remaining_len(struct parser* p) {
    return p->length - p->offset;
}

void parser_init(struct parser* p, size_t len, const char input[len]);

bool parser_test_ws(struct parser* p);

bool parser_skip_ws(struct parser* p);

// Match a [a-zA-Z0-9-]+
size_t parser_eat_word(struct parser* p);

#endif
