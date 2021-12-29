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
    'src/main.c',
    'src/connection.c',
    'src/interpreter.c',
    'src/command/command.c',
    'src/command/parser.c',
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
