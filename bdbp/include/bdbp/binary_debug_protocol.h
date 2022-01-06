#ifndef _BDBP_BINARY_DEBUG_PROTOCOL_H
#define _BDBP_BINARY_DEBUG_PROTOCOL_H

// General message format:
//
// | header (1 byte) | data length (1 byte) | data ('data length' bytes) |
//
// For packets going to the coprocessor (requests), the header consists of a command.
// For packets sent back by the coprocessor (responses), the header consists of a status.
// If a request was successfully completed, the device will respond with status SUCCESS
// and request-dependent data. Otherwise, one of the other status codes will be returned
// and the data field is empty.

enum bdbp_cmd {
    // Ping the device to see if it is online. Data field is empty, and length is 0.
    // Successful response has no data.
    BDBP_CMD_PING = 0x01,

    // Write to target memory. Data field consists of 2 + variable bytes: the address, and the data
    // to write.
    // Successful response has no data.
    BDBP_CMD_WRITE = 0x02,

    // Read from target memory. Data field consists of 3 bytes: the address to start reading from,
    // and the number of bytes to read.
    // Successfull response carries the bytes from the requested memory location.
    BDBP_CMD_READ = 0x03
};

enum bdbp_status {
    // Request was carried out successfully.
    // Response data depends on request.
    BDBP_STATUS_SUCCESS = 0x01,

    // Request header contained an invalid command.
    // Response data is empty.
    BDBP_STATUS_UNKNOWN_CMD = 0x02
};

// Definitions for offsets of packet fields.
#define BDBP_FIELD_HDR (0)
#define BDBP_FIELD_DATA_LEN (1)
#define BDBN_FIELD_DATA (2)

// Data field is 1 byte.
#define BDBP_MAX_DATA_LENGTH (255)

// 2 bytes for the header and data length, MAX_DATA_LENGTH bytes for the data itself.
#define BDBP_MAX_MSG_LENGTH (2 + BDBP_MAX_DATA_LENGTH)

#endif
