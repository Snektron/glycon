#ifndef _GLYDB_VALUE_H
#define _GLYDB_VALUE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct parser;

enum value_type {
    VALUE_TYPE_INT,
    VALUE_TYPE_STR,
    VALUE_TYPE_BOOL
};

union value_data {
    int64_t as_int;
    char* as_str;
    bool as_bool;
};

struct value {
    enum value_type type;
    union value_data data;
};

enum value_parse_status {
    VALUE_PARSE_SUCCESS,
    VALUE_PARSE_UNEXPECTED_END,
    VALUE_PARSE_UNEXPECTED_CHARACTER,
    VALUE_PARSE_INVALID_BOOL_LITERAL,
    VALUE_PARSE_INTEGER_OUT_OF_RANGE,
};

enum value_parse_status value_parse(union value_data* result, struct parser* p, enum value_type expected_type);

void value_free(const struct value* val);

const char* value_parse_status_to_str(enum value_parse_status status);

#endif
