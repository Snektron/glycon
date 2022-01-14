#ifndef _COMMON_BINARY_DEBUG_PROTOCOL_H
#define _COMMON_BINARY_DEBUG_PROTOCOL_H

// General message format:
//
// | HDR (1 byte) | DLEN (1 byte) | DATA ('DLEN' bytes) |
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
    // | 0x02 | 0x02 + var | ADDR (2 bytes) | DATA (DLEN - 2 bytes) |
    // Successful response has no data.
    BDBP_CMD_WRITE = 0x02,

    // Read from target memory. Data field consists of 3 bytes: the address to start reading from,
    // and the number of bytes to read.
    // | 0x03 | 0x03 | ADDR (2 byte) | AMT (1 byte) |
    // Successful response carries the bytes from the requested memory location.
    // | 0x01 | var | DATA (var bytes) |
    BDBP_CMD_READ = 0x03,

    // Write to target flash. Data consists of 2 + variable bytes: the address, and the data to write.
    // | 0x04 | 0x02 + var | ADDR (2 byte) | DATA (DLEN - 2 bytes) |
    // Successful response has no data.
    BDBP_CMD_WRITE_FLASH = 0x04,

    // Retrieve the flash software ID of the target. Takes no data.
    // | 0x05 | 0x00 |
    // Successful response carries the manufacterer ID and device ID of the flash chip.
    // | 0x01 | 0x02 | MFG ID (1 byte) | DEV ID (1 byte) |
    BDBP_CMD_FLASH_ID = 0x05,

    // Erase a single sector of the flash chip. Data field consists of an address; the sector of which
    // to erase. Sectors are 4 kilobytes and aligned to 4 kilobytes.
    // | 0x06 | 0x02 | ADDR (2 bytes) |
    // Successful response has no data.
    BDBP_CMD_ERASE_SECTOR = 0x06,

    // Erase the entire flash chip. Carries no data.
    // | 0x07 | 0x00 |
    // Successful response has no data.
    BDBP_CMD_ERASE_CHIP = 0x07,
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
#define BDBP_FIELD_DATA (2)

// Data field is 1 byte.
#define BDBP_MAX_DATA_LENGTH (255)

// The size of a packet with just the mandatory fields.
#define BDBP_MIN_MSG_LENGTH (2)
// 2 bytes for the header and data length, MAX_DATA_LENGTH bytes for the data itself.
#define BDBP_MAX_MSG_LENGTH (BDBP_MIN_MSG_LENGTH + BDBP_MAX_DATA_LENGTH)

#endif
