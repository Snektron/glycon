sources = [
    'src/bdbp_util.c',
    'src/buffer.c',
    'src/command.c',
    'src/connection.c',
    'src/debugger.c',
    'src/main.c',
    'src/parser.c',
    'src/target.c',
    'src/value.c',
    'src/commands/commands.c',
    'src/commands/connection.c',
    'src/commands/disassemble.c',
    'src/commands/flash.c',
    'src/commands/help.c',
    'src/commands/memory.c',
    'src/commands/ping.c',
    'src/commands/quit.c',
    'src/z80/disassemble.c',
    'src/z80/z80.c',
]

deps = [
    dependency('libeditline', native: true),
]

executable(
    'glydb',
    sources,
    dependencies: deps,
    native: true,
    include_directories: [common_inc, include_directories('src')],
)
