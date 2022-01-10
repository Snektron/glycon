sources = [
    'src/bdbp_util.c',
    'src/command.c',
    'src/connection.c',
    'src/debugger.c',
    'src/main.c',
    'src/parser.c',
    'src/value.c',
    'src/commands/commands.c',
    'src/commands/connection.c',
    'src/commands/flash.c',
    'src/commands/help.c',
    'src/commands/memory.c',
    'src/commands/ping.c',
    'src/commands/quit.c',
]

deps = [
    dependency('libeditline', native: true),
]

executable(
    'glydb',
    sources,
    dependencies: deps,
    native: true,
    include_directories: [bdbp_inc, include_directories('src')],
)
