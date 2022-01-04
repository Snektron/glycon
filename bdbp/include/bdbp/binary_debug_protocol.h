#ifndef _BDBP_BINARY_DEBUG_PROTOCOL_H
#define _BDBP_BINARY_DEBUG_PROTOCOL_H

enum bdbp_cmd {
    BDBP_CMD_PING = 0x01,
    BDBP_CMD_WRITE = 0x02,
    BDBP_CMD_READ = 0x03
};

enum bdbp_status {
    BDBP_STATUS_SUCCESS = 0x01,
    BDBP_STATUS_UNKNOWN_CMD = 0x02
};

#endif
