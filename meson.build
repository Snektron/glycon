project(
    'glycon',
    'c',
    default_options: [
        'c_std=gnu11',
        'b_lto=true',
        'buildtype=debugoptimized',
    ]
)

if not meson.is_cross_build()
    error('Glycon should be cross-compiled for the co-processor: meson build --cross-file cross/avr-atmega2560-cross.ini')
endif

add_project_arguments(
    ['-Wno-unused-function'],
    language: 'c',
)

subdir('common')
subdir('glyco')
subdir('glydb')
subdir('glyos')