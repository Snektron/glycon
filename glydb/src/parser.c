#include "parser.h"
#include <ctype.h>

void parser_init(struct parser* p, size_t len, const char input[len]) {
    p->input = input;
    p->length = len;
    p->offset = 0;
}

bool parser_test_ws(struct parser* p) {
    switch (parser_peek(p)) {
        case ' ':
        case '\t':
        case '\r':
        case '\f':
            return true;
        default:
            return false;
    }
}

bool parser_test_ws_or_end(struct parser* p) {
    return parser_is_at_end(p) || parser_test_ws(p);
}

bool parser_skip_ws(struct parser* p) {
    if (!parser_test_ws(p))
        return false;

    do {
        ++p->offset;
    } while (parser_test_ws(p));

    return true;
}

size_t parser_eat_word(struct parser* p) {
    if (parser_test_ws_or_end(p))
        return 0;

    ++p->offset;
    size_t len = 1;
    while (true) {
        if (parser_test_ws_or_end(p))
            return len;
        ++p->offset;
        ++len;
    }
}
