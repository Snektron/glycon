#ifndef _GLYDB_PROTOCOL_H
#define _GLYDB_PROTOCOL_H

enum glyco_cmd {
    GLYCO_CMD_PING = 0x01
};

enum glyco_status {
    GLYCO_STATUS_SUCCESS = 0x01,
    GLYCO_STATUS_UNKNOWN_CMD = 0x02
};

const char* glyco_status_to_string(enum glyco_status status);

#endif
