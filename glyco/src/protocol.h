#ifndef _GLYCO_PROTOCOL_H
#define _GLYCO_PROTOCOL_H

enum glyco_cmd {
    GLYCO_CMD_PING = 0x01
};

enum glyco_status {
    GLYCO_STATUS_SUCCESS = 0x01,
    GLYCO_STATUS_UNKNOWN_CMD = 0x02
};

#endif
