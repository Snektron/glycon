#include "commands/commands.h"
#include "debugger.h"
#include "bdbp/binary_debug_protocol.h"
#include "bdbp_util.h"

#include <stdio.h>

static void memory_write(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    uint8_t data = args->positionals[1].as_int;

    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    bdbp_pkt_init(buf, BDBP_CMD_WRITE);
    bdbp_pkt_append_u16(buf, address);
    bdbp_pkt_append_u8(buf, data);
    debugger_exec_cmd(dbg, buf);
}

static void memory_read(struct debugger* dbg, const struct cmd_parse_result* args) {
    uint16_t address = args->positionals[0].as_int;
    size_t amt = args->positionals_len > 1 ? args->positionals[1].as_int : 1;

    uint8_t bytes_per_line = 16;
    // Round down the number of bytes were going to handle per packet to a multiple of the line
    // size so that we don't need to handle them weird.
    uint8_t bytes_per_packet = (BDBP_MAX_DATA_LENGTH - 3) / bytes_per_line * bytes_per_line;

    uint8_t buf[BDBP_MAX_MSG_LENGTH];
    for (size_t i = 0; i < amt; i += bytes_per_packet) {
        size_t bytes_left = amt - i;
        uint8_t packet_bytes = bytes_left > bytes_per_packet ? bytes_per_packet : bytes_left;
        bdbp_pkt_init(buf, BDBP_CMD_READ);
        bdbp_pkt_append_u16(buf, address + i);
        bdbp_pkt_append_u8(buf, packet_bytes);
        if (debugger_exec_cmd(dbg, buf))
            return;

        for (size_t j = 0; j < packet_bytes; j += bytes_per_line) {
            printf("%04X:", (uint16_t)(address + i + j));
            for (uint8_t k = 0; k < bytes_per_line && k + j < packet_bytes; ++k) {
                printf(" %02X", buf[BDBP_FIELD_DATA + j + k]);
            }
            puts("");
        }
    }
}

static const struct cmd* memory_commands[] = {
    &(struct cmd){CMD_TYPE_LEAF, "write", "Write to target memory.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to write to."},
            {VALUE_TYPE_INT, "value", "Value to write."},
            {}
        },
        .payload = memory_write
    }}},
    &(struct cmd){CMD_TYPE_LEAF, "read", "Read from target memory.", {.leaf = {
        .options = NULL,
        .positionals = (struct cmd_positional[]){
            {VALUE_TYPE_INT, "address", "The address to read from."},
            {VALUE_TYPE_INT, "amount", "The number of bytes to read (default: 1).", CMD_OPTIONAL},
            {}
        },
        .payload = memory_read
    }}},
    NULL
};

const struct cmd command_memory = {
    .type = CMD_TYPE_DIRECTORY,
    .name = "memory",
    .help = "Manipulate target memory",
    {.directory = {memory_commands}}
};
