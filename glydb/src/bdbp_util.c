#include "bdbp_util.h"

const char* bdbp_status_to_string(enum bdbp_status status) {
    switch (status) {
        case BDBP_STATUS_SUCCESS:
            return "Success";
        case BDBP_STATUS_UNKNOWN_CMD:
            return "Unknown command";
        default:
            return "(Invalid status)";
    }
}
