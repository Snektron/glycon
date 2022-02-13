#include "value.h"
#include "parser.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

// Parse an integer value.
static enum value_parse_status parse_int(int64_t* result_ptr, struct parser* p) {
    // TODO: Expressions.
    // TODO: Reset parser to start of int on error.
    parser_skip_ws(p);
    if (parser_is_at_end(p)) {
        return VALUE_PARSE_UNEXPECTED_END;
    }

    bool negate = false;
    if (parser_peek(p) == '-') {
        negate = true;
        ++p->offset;
    }

    uint64_t base = 10;
    if (parser_peek(p) == '0') {
        // This character won't have effect on the final value even if its not part
        // of a radix prefix, so we can safely ignore it.
        ++p->offset;
        switch (parser_peek(p)) {
            case 'x':
                base = 16;
                ++p->offset;
                break;
            case 'o':
                base = 8;
                ++p->offset;
                break;
            case 'b':
                base = 2;
                ++p->offset;
                break;
            default:
                --p->offset;
                break;
        }
    }

    if (parser_test_ws_or_end(p))
        return VALUE_PARSE_UNEXPECTED_CHARACTER;

    uint64_t max = -(uint64_t) INT64_MIN;
    uint64_t result = 0;
    while (!parser_test_ws_or_end(p)) {
        int c = parser_peek(p);
        uint64_t digit;
        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'A' && c <= 'Z') {
            digit = c - 'A' + 10;
        } else if (c >= 'a' && c <= 'z') {
            digit = c - 'a' + 10;
        } else {
            return VALUE_PARSE_UNEXPECTED_CHARACTER;
        }

        if (digit >= base)
            return VALUE_PARSE_UNEXPECTED_CHARACTER;
        if (result != 0 && base > max / result)
            return VALUE_PARSE_INTEGER_OUT_OF_RANGE;
        result *= base;
        if (digit > max - result)
            return VALUE_PARSE_INTEGER_OUT_OF_RANGE;
        result += digit;
        ++p->offset;
    }

    if (negate) {
        *result_ptr = (int64_t) -result;
    } else if (result == max) {
        // Would overflow
        return VALUE_PARSE_INTEGER_OUT_OF_RANGE;
    } else {
        *result_ptr = (int64_t) result;
    }

    return VALUE_PARSE_SUCCESS;
}

// Parse a string value.
static enum value_parse_status parse_str(char** result_ptr, struct parser* p) {
    parser_skip_ws(p);
    if (parser_is_at_end(p)) {
        return VALUE_PARSE_UNEXPECTED_END;
    }

    const char* str = parser_remaining(p);
    size_t len = 0;
    while (true) {
        int c = parser_peek(p);
        if (c == '\\') {
            ++p->offset;
            if (parser_is_at_end(p))
                return VALUE_PARSE_UNEXPECTED_END;
        } else if (parser_test_ws_or_end(p)) {
            break;
        }

        ++p->offset;
        ++len;
    }

    char* result = malloc(len + 1);
    for (size_t i = 0, j = 0; i < len; ++i, ++j) {
        result[i] = str[str[j] == '\\' ? ++j : j];
    }
    result[len] = 0;

    *result_ptr = result;
    return VALUE_PARSE_SUCCESS;
}

// Parse a boolean value.
static enum value_parse_status parse_bool(bool* result_ptr, struct parser* p) {
    parser_skip_ws(p);
    if (parser_is_at_end(p)) {
        return VALUE_PARSE_UNEXPECTED_END;
    }

    const char* word = parser_remaining(p);
    size_t word_len = parser_eat_word(p);
    if (word_len == 0) {
        return VALUE_PARSE_UNEXPECTED_CHARACTER;
    }

    if (strncmp("true", word, word_len) == 0) {
        *result_ptr = true;
        return VALUE_PARSE_SUCCESS;
    } else if (strncmp("false", word, word_len) == 0) {
        *result_ptr = false;
        return VALUE_PARSE_SUCCESS;
    }

    // Rewind the parser so that it points to the start of the word.
    p->offset -= word_len;
    return VALUE_PARSE_INVALID_BOOL_LITERAL;
}

enum value_parse_status value_parse(union value_data* result, struct parser* p, enum value_type expected_type) {
    switch (expected_type) {
        case VALUE_TYPE_INT:
            return parse_int(&result->as_int, p);
        case VALUE_TYPE_STR:
            return parse_str(&result->as_str, p);
        case VALUE_TYPE_BOOL:
            return parse_bool(&result->as_bool, p);
    }
}

void value_free(const struct value* val) {
    if (val->type == VALUE_TYPE_STR) {
        free(val->data.as_str);
    }
}

const char* value_parse_status_to_str(enum value_parse_status status) {
    switch (status) {
        case VALUE_PARSE_SUCCESS:
            return "Success";
        case VALUE_PARSE_UNEXPECTED_END:
            return "Unexpected end of input";
        case VALUE_PARSE_UNEXPECTED_CHARACTER:
            return "Unexpected character";
        case VALUE_PARSE_INVALID_BOOL_LITERAL:
            return "Invalid 'bool' literal";
        case VALUE_PARSE_INTEGER_OUT_OF_RANGE:
            return "Integer out of range";
    }
}
