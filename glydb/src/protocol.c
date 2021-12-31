#include "protocol.h"

const char* glyco_status_to_string(enum glyco_status status) {
    switch (status) {
        case GLYCO_STATUS_SUCCESS:
            return "Success";
        case GLYCO_STATUS_UNKNOWN_CMD:
            return "Unknown command";
        default:
            return "(Invalid status)";
    }
}
