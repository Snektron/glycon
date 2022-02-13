#ifndef _GLYDB_VALUE_H
#define _GLYDB_VALUE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

struct parser;

// Possible value variants.
enum value_type {
    VALUE_TYPE_INT,
    VALUE_TYPE_STR,
    VALUE_TYPE_BOOL
};

// Union that can be used to store different values - the actual type is stored externally.
union value_data {
    int64_t as_int;
    char* as_str;
    bool as_bool;
};

// This structure can be used to store a dynamic value - either an integer, string, or bool.
struct value {
    enum value_type type;
    union value_data data;
};

// Possible status codes that are returned by `value_parse`.
enum value_parse_status {
    VALUE_PARSE_SUCCESS,
    VALUE_PARSE_UNEXPECTED_END,
    VALUE_PARSE_UNEXPECTED_CHARACTER,
    VALUE_PARSE_INVALID_BOOL_LITERAL,
    VALUE_PARSE_INTEGER_OUT_OF_RANGE,
};

// Parse a value of type `expected type`, using the given parser. The result is stored in `result`.
// Returns a parse status describing how the parsing went.
enum value_parse_status value_parse(union value_data* result, struct parser* p, enum value_type expected_type);

// Free any data owned by a particular value.
void value_free(const struct value* val);

// Translate a particular parse status into a string that describes it.
const char* value_parse_status_to_str(enum value_parse_status status);

#endif
