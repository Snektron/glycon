project(
    'glydb',
    'c',
    default_options: [
        'c_std=gnu11',
        'b_lto=true',
        'buildtype=debugoptimized',
    ]
)

sources = [
    'src/connection.c',
    'src/debugger.c',
    'src/main.c',
    'src/parser.c',
    'src/command.c',
    'src/commands/commands.c',
    'src/commands/connection.c',
    'src/commands/help.c',
    'src/commands/memory.c',
    'src/commands/quit.c',
]

deps = [
    dependency('libeditline'),
]

executable(
    'glydb',
    sources,
    dependencies: deps,
    include_directories: include_directories('src')
)
