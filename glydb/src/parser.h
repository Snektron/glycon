#ifndef _GLYDB_PARSER_H
#define _GLYDB_PARSER_H

#include <stddef.h>
#include <stdbool.h>

// A generic parser structure.
struct parser {
    // The input that is to be parsed.
    const char* input;

    // Total length of the input.
    size_t length;

    // The current progress.
    size_t offset;
};

// Return whether there is anything left to parse.
inline int parser_is_at_end(struct parser* p) {
    return p->offset >= p->length;
}

// Return the next character without advancing the parser. Returns -1 if there is no such character.
inline int parser_peek(struct parser* p) {
    if (parser_is_at_end(p))
        return -1;
    return p->input[p->offset];
}

// Return the next character, and advance the parser to the next character.
// Returns -1 if there is no such character.
inline int parser_consume(struct parser* p) {
    if (parser_is_at_end(p))
        return -1;
    return p->input[p->offset++];
}

// Compare the next character with `expected`. If they are the same, the parser advances to the next
// character and returns true. Otherwise, this function returns false.
inline bool parser_eat(struct parser* p, char expected) {
    int c = parser_peek(p);
    if (c == expected) {
        ++p->offset;
        return true;
    }
    return false;
}

// Return a pointer to the remaining input, the part that is not yet parsed.
inline const char* parser_remaining(struct parser* p) {
    return &p->input[p->offset];
}

// Return the number of characters that haven't been parsed yet. This is the length
// of the string returned by `parser_remaining()`.
inline size_t parser_remaining_len(struct parser* p) {
    return p->length - p->offset;
}

// Initialize a parser with a particular input string.
void parser_init(struct parser* p, size_t len, const char input[len]);

// Return whether the parser is currently at whitespace.
// Whitespace consists of space, tab, cariage return, and feed.
bool parser_test_ws(struct parser* p);

// Return whether the parser is currently at whitespace or the end of the input.
// Whitespace consists of space, tab, cariage return, and feed.
bool parser_test_ws_or_end(struct parser* p);

// Skip any whitespace that the parser is currently at. Returns whether any
// was skipped at all.
// Whitespace consists of space, tab, cariage return, and feed.
bool parser_skip_ws(struct parser* p);

// Match any whitespace-delimited word.
// Whitespace consists of space, tab, cariage return, and feed.
size_t parser_eat_word(struct parser* p);

// Debug-utility to dump the current state of the parser.
void parser_dump_state(struct parser* p);

#endif
