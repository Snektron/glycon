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
]

executable(
    'glydb',
    sources
)
