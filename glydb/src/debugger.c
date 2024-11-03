#include "debugger.h"
#include "command.h"
#include "parser.h"
#include "bdbp_util.h"
#include "commands/commands.h"

#include "common/binary_debug_protocol.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <assert.h>

#include <editline.h>

static const char* prompt = "(glydb) ";

void debugger_init(struct debugger* dbg, const char* initial_port) {
    dbg->quit = false;
    conn_init(&dbg->conn);
    dbg->scratch = malloc(GLYCON_ADDRSPACE_SIZE);

    if (initial_port) {
        (void) subcommand_open(dbg, initial_port);
    }
}

void debugger_deinit(struct debugger* dbg) {
    conn_close(&dbg->conn);
    free(dbg->scratch);
}

void debugger_do_line(struct debugger* dbg, size_t len, const char line[]) {
    struct parser p;
    parser_init(&p, len, line);

    struct cmd_parse_result result;
    if (cmd_parse(&result, &p, commands) && result.matched_command) {
        switch (result.matched_command->type) {
            case CMD_TYPE_LEAF: {
                command_handler_t handler = result.matched_command->leaf.payload;
                handler(dbg, &result);
                break;
            }
            case CMD_TYPE_DIRECTORY:
                // TODO: Print help for the directory itself?
                break;
        }
    }

    cmd_parse_result_deinit(&result);
}

void debugger_repl(struct debugger* dbg) {
    rl_initialize();
    dbg->quit = false;

    while (!dbg->quit) {
        char* line = readline(prompt);
        if (!line) {
            return;
        } else {
            size_t len = strlen(line);
            debugger_do_line(dbg, len, line);
            free(line);
        }
    }
    rl_uninitialize();
}

bool debugger_require_connection(struct debugger* dbg) {
    if (!conn_is_open(&dbg->conn)) {
        debugger_print_error(dbg, "No active connection. Connect to a device using `connection open`.");
        return true;
    }

    return false;
}

void debugger_print_error(struct debugger* dbg, const char* fmt, ...) {
    (void) dbg;
    va_list args;
    va_start(args, fmt);
    printf("error: ");
    vprintf(fmt, args);
    puts("");
    va_end(args);
}

static bool load_bin(struct debugger* dbg, const struct debugger_load_file_options* opts, struct debugger_write_op** ops, FILE* f, uint8_t* buffer) {
    if (fseek(f, 0, SEEK_END) || ferror(f)) {
        debugger_print_error(dbg, "Failed to seek.");
        goto err_close_file;
    }

    long size = ftell(f);
    if (size < 0) {
        debugger_print_error(dbg, "Failed to tell: %s.", strerror(errno));
        goto err_close_file;
    }

    if (size > GLYCON_ADDRSPACE_SIZE) {
        debugger_print_error(dbg, "Binary file '%s' overflows address space.", opts->path);
        goto err_close_file;
    }

    rewind(f);
    if (fread(buffer, 1, size, f) < 0 || ferror(f)) {
        debugger_print_error(dbg, "Failed to read '%s'", opts->path);
        goto err_close_file;
    }

    // Second op indicates end.
    *ops = calloc(2, sizeof(struct debugger_write_op));
    assert(*ops);

    (*ops)[0].address = opts->relocation;
    (*ops)[0].len = size;
    fclose(f);
    return false;

err_close_file:
    fclose(f);
    return true;
}

static bool load_ihex(struct debugger* dbg, const struct debugger_load_file_options* opts, struct debugger_write_op** ops, FILE* f, uint8_t* buffer) {
    (void) opts;
    (void) buffer;
    fclose(f);
    debugger_print_error(dbg, "TODO: ihex loading.");
    return true;
}

bool debugger_load_file(struct debugger* dbg, const struct debugger_load_file_options* opts, struct debugger_write_op** ops, uint8_t* buffer) {
    const char* ext = opts->ext_override ? opts->ext_override  : strrchr(opts->path, '.');
    if (!ext) {
        debugger_print_error(dbg, "Unable to infer file type from '%s'.", opts->path);
        return true;
    }

    FILE* f = fopen(opts->path, "rb");
    if (!f) {
        debugger_print_error(dbg, "Failed to open file '%s': %s.", opts->path, strerror(errno));
        return true;
    }

    ++ext;
    if (strcmp(ext, "bin") == 0) {
        return load_bin(dbg, opts, ops, f, buffer);
    } else if (strcmp(ext, "ihx") == 0) {
        return load_ihex(dbg, opts, ops, f, buffer);
    } else {
        debugger_print_error(dbg, "Unknown file type '%s'.", ext);
        fclose(f);
        return true;
    }
}
