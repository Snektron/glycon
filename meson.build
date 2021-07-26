project(
    'Glycon',
    'c',
    default_options: [
        'c_std=gnu11',
        'b_lto=true',
        'buildtype=minsize',
    ]
)

if not meson.is_cross_build()
    error('Glycon controller should be cross-compiled: meson glycon build --cross-file glycon/coprocessor/avr-atmega2560-cross.ini')
endif

subdir('coprocessor')
