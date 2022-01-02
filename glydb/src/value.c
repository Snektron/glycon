#include "value.h"
#include "parser.h"

#include <stdlib.h>
#include <string.h>

static enum value_parse_status parse_int(int64_t* result_ptr, struct parser* p) {
    // TODO: Expressions.
    // TODO: Reset parser to start of int on error.
    parser_skip_ws(p);
    if (parser_is_at_end(p)) {
        return VALUE_PARSE_UNEXPECTED_END;
    }

    const char* value = parser_remaining(p);
    size_t len = parser_eat_word(p);
    if (len == 0) {
        return VALUE_PARSE_UNEXPECTED_CHARACTER;
    }

    bool negate = false;
    if (value[0] == '-') {
        negate = true;
        --len;
        ++value;
    }

    int base = 10;
    if (len >= 2 && value[0] == '0') {
        switch (value[1]) {
            case 'x':
                base = 16;
                break;
            case 'o':
                base = 8;
                break;
            case 'b':
                base = 2;
                break;
            default:
                break;
        }
        switch (value[1]) {
            case 'x':
            case 'o':
            case 'b':
                len -= 2;
                value += 2;
                break;
            default:
                break;
        }
    }

    uint64_t max = -(uint64_t) INT64_MIN;
    uint64_t result = 0;
    for (size_t i = 0; i < len; ++i) {
        if (10 > max / result)
            return VALUE_PARSE_INTEGER_OUT_OF_RANGE;
        result *= 10;
        uint64_t digit = value[i] - '0';
        if (digit > max - result)
            return VALUE_PARSE_INTEGER_OUT_OF_RANGE;
        result += digit;
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

    char* result = malloc(len);
    for (size_t i = 0, j = 0; i < len; ++i) {
        result[i] = str[str[j] == '\\' ? ++j : j];
    }

    *result_ptr = result;
    return VALUE_PARSE_SUCCESS;
}

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
