#include "command/parser.h"
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

bool parser_skip_ws(struct parser* p) {
    if (!parser_test_ws(p))
        return false;

    do {
        ++p->offset;
    } while (parser_test_ws(p));

    return true;
}

size_t parser_eat_word(struct parser* p) {
    int c = parser_peek(p);
    if (!isalnum(c) && c != '-')
        return 0;

    ++p->offset;
    size_t len = 1;
    while (true) {
        c = parser_peek(p);
        if (!isalnum(c) && c != '-')
            return len;
        ++p->offset;
        ++len;
    }
}
